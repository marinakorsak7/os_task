#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <errno.h>
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

void process_client_request(const char* client_fifo) {
    int client_fd;
    while ((client_fd = open(client_fifo, O_WRONLY)) == -1) {
        if (errno != ENOENT) {
            perror("open client_fifo");
            return;
        }
        usleep(10000);
    }

    char buffer[MAX_BUFFER_SIZE];
    int bytes_read = receive_data(client_fd, buffer, MAX_BUFFER_SIZE);
    if (bytes_read > 0) {
        buffer[bytes_read] = '\0';
        char* command = strtok(buffer, " ");
        if (strcmp(command, "set") == 0) {
            char* key = strtok(NULL, " ");
            char* value = strtok(NULL, "\n");
            int index = search_key(key);
            if (index == -1) {
                strcpy(kv_store[kv_count].key, key);
                strcpy(kv_store[kv_count].value, value);
                kv_count++;
            } else {
                strcpy(kv_store[index].value, value);
            }
        } else if (strcmp(command, "get") == 0) {
            char* key = strtok(NULL, "\n");
            int index = search_key(key);
            char response[MAX_BUFFER_SIZE];
            if (index == -1) {
                snprintf(response, sizeof(response), "Key not found");
            } else {
                snprintf(response, sizeof(response), "Value: %s", kv_store[index].value);
            }
            send_data(client_fd, response);
        }
    }
    close(client_fd);
}

void cleanup() {
    unlink(SERVER_PIPE);
}

int main() {
    signal(SIGTERM, signal_handler);

    if (access(SERVER_PIPE, F_OK) == 0) {
        unlink(SERVER_PIPE);
    }

    if (mkfifo(SERVER_PIPE, 0666) == -1) {
        perror("mkfifo");
        exit(EXIT_FAILURE);
    }

    printf("Server is running...\n");

    int server_fd = open(SERVER_PIPE, O_RDONLY);
    if (server_fd == -1) {
        perror("open server_fifo");
        cleanup();
        exit(EXIT_FAILURE);
    }

    while (server_active) {
        char buffer[MAX_BUFFER_SIZE];
        int bytes_read = receive_data(server_fd, buffer, MAX_BUFFER_SIZE);
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';
            printf("Received request: %s\n", buffer);
            char* client_fifo = buffer + 8;
            process_client_request(client_fifo);
        }
    }

    close(server_fd);
    cleanup();
    return 0;
}
