#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <errno.h>
#include <pthread.h>
#include "kv_lib.h"

#define SERVER_PIPE "/tmp/server.fifo"
#define MAX_BUFFER_SIZE 256

typedef struct {
    char key[50];
    char value[100];
} KeyValuePair;

KeyValuePair kv_store[100];
int kv_count = 0;
volatile sig_atomic_t server_active = 1;

void signal_handler(int sig) {
    (void)sig;
    server_active = 0;
    unlink(SERVER_PIPE);
    printf("Server shutting down.\n");
    exit(0);
}

int search_key(const char* key) {
    for (int i = 0; i < kv_count; i++) {
        if (strcmp(kv_store[i].key, key) == 0) {
            return i;
        }
    }
    return -1;
}

void* process_client_request(void* arg) {
    char* request = (char*)arg;
    char* command = strtok(request, " ");  
    char* key = strtok(NULL, " ");         
    char* value = NULL;                   
    char* client_fifo;

    if (strcmp(command, "set") == 0) {
        value = strtok(NULL, " ");         
        client_fifo = strtok(NULL, "");    
    } else {
        client_fifo = strtok(NULL, "");   
    }

    if (command == NULL || key == NULL || client_fifo == NULL) {
        fprintf(stderr, "Invalid request format\n");
        free(request);
        return NULL;
    }

    printf("Processing request: %s %s %s %s\n", command, key, value ? value : "(no value)", client_fifo);

    if (access(client_fifo, F_OK) != -1) {
        int client_fd = open(client_fifo, O_WRONLY);
        if (client_fd == -1) {
            perror("open client_fifo");
            free(request);
            return NULL;
        }

        char response[MAX_BUFFER_SIZE];
        if (strcmp(command, "set") == 0 && value != NULL) {
            int index = search_key(key);
            if (index == -1) {
                if (kv_count < 100) {
                    strcpy(kv_store[kv_count].key, key);
                    strcpy(kv_store[kv_count].value, value);
                    kv_count++;
                    snprintf(response, MAX_BUFFER_SIZE, "Key: %s set to Value: %s", key, value);
                } else {
                    snprintf(response, MAX_BUFFER_SIZE, "Key-value store is full");
                }
            } else {
                strcpy(kv_store[index].value, value);
                snprintf(response, MAX_BUFFER_SIZE, "Updated Key: %s with Value: %s", key, value);
            }
        } else if (strcmp(command, "get") == 0) {
            int index = search_key(key);
            if (index == -1) {
                snprintf(response, MAX_BUFFER_SIZE, "Key not found");
            } else {
                snprintf(response, MAX_BUFFER_SIZE, "Value: %s", kv_store[index].value);
            }
        }

        if (write(client_fd, response, strlen(response) + 1) == -1) {
            perror("write to client_fifo");
        }

        close(client_fd);
    } else {
        printf("Client FIFO does not exist: %s\n", client_fifo);
    }

    free(request);
    return NULL;
}




void cleanup() {
    unlink(SERVER_PIPE);
}

int main() {
    signal(SIGTERM, signal_handler);

    if (mkfifo(SERVER_PIPE, 0666) == -1) {
        perror("mkfifo");
        exit(EXIT_FAILURE);
    }

    int server_fd = open(SERVER_PIPE, O_RDONLY | O_NONBLOCK);
    if (server_fd == -1) {
        perror("open server_fifo");
        cleanup();
        exit(EXIT_FAILURE);
    }

    printf("Server is running...\n");

    while (server_active) {
        char buffer[MAX_BUFFER_SIZE];
        int bytes_read = read(server_fd, buffer, MAX_BUFFER_SIZE);
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';
            char* request = strdup(buffer);
            pthread_t thread;
            if (pthread_create(&thread, NULL, process_client_request, request) != 0) {
                perror("pthread_create");
                free(request);
            } else {
                pthread_detach(thread);
            }
        } else if (bytes_read == -1 && errno != EAGAIN) {
            perror("read error");
        }
    }

    close(server_fd);
    cleanup();
    return 0;
}

