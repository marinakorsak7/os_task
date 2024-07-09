#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <signal.h>
#include <errno.h>
#include "kv_lib.h"

#define SERVER_SOCKET_PATH "/tmp/server_socket"
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
    unlink(SERVER_SOCKET_PATH);
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

void process_client_request(int client_sockfd) {
    char buffer[MAX_BUFFER_SIZE];
    int bytes_received = read(client_sockfd, buffer, MAX_BUFFER_SIZE);

    if (bytes_received > 0) {
        buffer[bytes_received] = '\0';
        printf("Processing request: %s\n", buffer);

        char* command = strtok(buffer, " ");
        if (command == NULL) {
            return;
        }

        if (strcmp(command, "set") == 0) {
            char* key = strtok(NULL, " ");
            char* value = strtok(NULL, "\n");
            if (key != NULL && value != NULL) {
                int index = search_key(key);
                if (index == -1) {
                    strcpy(kv_store[kv_count].key, key);
                    strcpy(kv_store[kv_count].value, value);
                    kv_count++;
                } else {
                    strcpy(kv_store[index].value, value);
                }
                printf("Set key: %s, value: %s\n", key, value);
                write(client_sockfd, "OK", 3);
            }
        } else if (strcmp(command, "get") == 0) {
            char* key = strtok(NULL, "\n");
            if (key != NULL) {
                int index = search_key(key);
                char response[MAX_BUFFER_SIZE];
                if (index == -1) {
                    snprintf(response, sizeof(response), "Key not found");
                } else {
                    snprintf(response, sizeof(response), "Value: %s", kv_store[index].value);
                }
                write(client_sockfd, response, strlen(response) + 1);
                printf("Get key: %s, response: %s\n", key, response);
            }
        }
    }

    close(client_sockfd);
}

void cleanup() {
    unlink(SERVER_SOCKET_PATH);
}

int main() {
    signal(SIGTERM, signal_handler);

    int server_sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_sockfd == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SERVER_SOCKET_PATH, sizeof(addr.sun_path) - 1);

    unlink(SERVER_SOCKET_PATH);
    if (bind(server_sockfd, (struct sockaddr *)&addr, sizeof(struct sockaddr_un)) == -1) {
        perror("bind");
        close(server_sockfd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_sockfd, 5) == -1) {
        perror("listen");
        close(server_sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Server is running...\n");

    while (server_active) {
        int client_sockfd = accept(server_sockfd, NULL, NULL);
        if (client_sockfd == -1) {
            if (errno == EINTR) {
                continue;
            } else {
                perror("accept");
                break;
            }
        }

        process_client_request(client_sockfd);
    }

    close(server_sockfd);
    cleanup();
    return 0;
}

