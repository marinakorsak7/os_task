#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include "kv_lib.h"

#define SERVER_PIPE "/tmp/server.fifo"
#define MAX_BUF_SIZE 256

void print_usage(char *program_name) {
    fprintf(stderr, "Usage: %s <command>\n", program_name);
}

int create_client_fifo(char *client_fifo) {
    sprintf(client_fifo, "/tmp/client_%d.fifo", getpid());
    if (mkfifo(client_fifo, 0666) == -1) {
        perror("mkfifo");
        return -1;
    }
    printf("Created client FIFO: %s\n", client_fifo);
    return 0;
}

int open_fifos(const char *client_fifo, int *client_fd, int *server_fd) {
    *server_fd = open(SERVER_PIPE, O_WRONLY);
    if (*server_fd == -1) {
        perror("open server_fifo");
        return -1;
    }
    printf("Opened server FIFO: %s\n", SERVER_PIPE);

    *client_fd = open(client_fifo, O_RDONLY | O_NONBLOCK);
    if (*client_fd == -1) {
        perror("open client_fifo");
        close(*server_fd);
        return -1;
    }
    printf("Opened client FIFO: %s\n", client_fifo);

    return 0;
}

int transmit_command(int server_fd, const char *command, const char *client_fifo) {
    char message[MAX_BUF_SIZE];
    snprintf(message, MAX_BUF_SIZE, "%s %s", command, client_fifo);
    if (write(server_fd, message, strlen(message) + 1) == -1) {
        perror("write to server_fifo");
        return -1;
    }
    printf("Transmitted command: %s\n", message);
    return 0;
}

void receive_response(int client_fd) {
    char response[MAX_BUF_SIZE];
    int bytes_received;

    printf("Receiving response...\n");

    while ((bytes_received = read(client_fd, response, MAX_BUF_SIZE)) > 0) {
        response[bytes_received] = '\0';
        printf("Server response: %s\n", response);
    }

    if (bytes_received == -1) {
        perror("read");
    } else if (bytes_received == 0) {
        printf("Server closed the connection.\n");
    }
}

void cleanup(int client_fd, const char *client_fifo) {
    close(client_fd);
    unlink(client_fifo);
    printf("Closed and removed client FIFO: %s\n", client_fifo);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        print_usage(argv[0]);
        exit(EXIT_FAILURE);
    }

    char client_fifo[MAX_BUF_SIZE];
    int client_fd, server_fd;

    if (create_client_fifo(client_fifo) == -1) {
        exit(EXIT_FAILURE);
    }

    if (open_fifos(client_fifo, &client_fd, &server_fd) == -1) {
        unlink(client_fifo);
        exit(EXIT_FAILURE);
    }

    if (transmit_command(server_fd, argv[1], client_fifo) == -1) {
        cleanup(client_fd, client_fifo);
        exit(EXIT_FAILURE);
    }

    sleep(1); 

    if (strncmp(argv[1], "get", 3) == 0 || strncmp(argv[1], "set", 3) == 0) {
        receive_response(client_fd);
    }

    cleanup(client_fd, client_fifo);

    return 0;
}

