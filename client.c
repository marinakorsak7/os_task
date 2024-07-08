#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "kv_lib.h"

#define SERVER_PIPE "/tmp/server.fifo"
#define MAX_BUF_SIZE 256

void print_usage(char *program_name) {
    fprintf(stderr, "Usage: %s <command>\n", program_name);
}

int create_client_fifo(char *client_fifo) {
    if (establish_connection(SERVER_PIPE, client_fifo) == -1) {
        return -1;
    }
    return 0;
}

int open_fifos(const char *client_fifo, int *client_fd, int *server_fd) {
    *client_fd = open(client_fifo, O_RDONLY);
    *server_fd = open(SERVER_PIPE, O_WRONLY);

    if (*server_fd == -1 || *client_fd == -1) {
        perror("open");
        if (*client_fd != -1) close(*client_fd);
        return -1;
    }
    return 0;
}

int transmit_command(int server_fd, const char *command) {
    if (send_data(server_fd, command) == -1) {
        perror("send_data");
        return -1;
    }
    return 0;
}

void receive_response(int client_fd) {
    char response[MAX_BUF_SIZE];
    int bytes_received = receive_data(client_fd, response, MAX_BUF_SIZE);

    if (bytes_received > 0) {
        response[bytes_received] = '\0';
        printf("Server response: %s\n", response);
    }
}

void cleanup(int client_fd, const char *client_fifo) {
    terminate_connection(client_fd);
    unlink(client_fifo);
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

    if (transmit_command(server_fd, argv[1]) == -1) {
        cleanup(client_fd, client_fifo);
        exit(EXIT_FAILURE);
    }

    receive_response(client_fd);
    cleanup(client_fd, client_fifo);

    return 0;
}
