#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>

#define SERVER_SOCKET_PATH "/tmp/server_socket"
#define CLIENT_SOCKET_TEMPLATE "/tmp/client_socket_%d"
#define MAX_BUF_SIZE 256

void print_usage(char *program_name) {
    fprintf(stderr, "Usage: %s <command>\n", program_name);
}

int create_client_socket(char *client_socket_path) {
    snprintf(client_socket_path, MAX_BUF_SIZE, CLIENT_SOCKET_TEMPLATE, getpid());
    int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("socket");
        return -1;
    }
    
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, client_socket_path, sizeof(addr.sun_path) - 1);

    unlink(client_socket_path);
    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(struct sockaddr_un)) == -1) {
        perror("bind");
        close(sockfd);
        return -1;
    }

    printf("Created client socket: %s\n", client_socket_path);
    return sockfd;
}

int connect_to_server(int sockfd) {
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SERVER_SOCKET_PATH, sizeof(addr.sun_path) - 1);

    if (connect(sockfd, (struct sockaddr *)&addr, sizeof(struct sockaddr_un)) == -1) {
        perror("connect");
        return -1;
    }

    printf("Connected to server socket: %s\n", SERVER_SOCKET_PATH);
    return 0;
}

int transmit_command(int sockfd, const char *command) {
    if (write(sockfd, command, strlen(command) + 1) == -1) {
        perror("write");
        return -1;
    }
    printf("Transmitted command: %s\n", command);
    return 0;
}

void receive_response(int sockfd) {
    char response[MAX_BUF_SIZE];
    int bytes_received;

    bytes_received = read(sockfd, response, MAX_BUF_SIZE);
    if (bytes_received > 0) {
        response[bytes_received] = '\0';
        printf("Server response: %s\n", response);
    } else if (bytes_received == -1) {
        perror("read");
    }
}

void cleanup(int sockfd, const char *client_socket_path) {
    close(sockfd);
    unlink(client_socket_path);
    printf("Closed and removed client socket: %s\n", client_socket_path);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        print_usage(argv[0]);
        exit(EXIT_FAILURE);
    }

    char client_socket_path[MAX_BUF_SIZE];
    int client_sockfd;

    client_sockfd = create_client_socket(client_socket_path);
    if (client_sockfd == -1) {
        exit(EXIT_FAILURE);
    }

    if (connect_to_server(client_sockfd) == -1) {
        cleanup(client_sockfd, client_socket_path);
        exit(EXIT_FAILURE);
    }

    if (transmit_command(client_sockfd, argv[1]) == -1) {
        cleanup(client_sockfd, client_socket_path);
        exit(EXIT_FAILURE);
    }

    receive_response(client_sockfd);
    cleanup(client_sockfd, client_socket_path);

    return 0;
}

