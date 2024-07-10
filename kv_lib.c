#include "kv_lib.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

/**
 * Establishes a connection to the server.
 * 
 * @param server_fifo The path to the server FIFO (named pipe).
 * @param client_fifo The buffer to store the path to the client FIFO (named pipe).
 * @return 0 on success, or -1 on failure.
 */
int establish_connection(const char* server_fifo, char* client_fifo) {
    sprintf(client_fifo, "/tmp/client_%d.fifo", getpid());
    if (mkfifo(client_fifo, 0666) == -1) {
        perror("mkfifo");
        return -1;
    }
    int server_fd = open(server_fifo, O_WRONLY);
    if (server_fd == -1) {
        perror("open server_fifo");
        unlink(client_fifo);
        return -1;
    }
    char connect_msg[256] = {0};  // Initialize the buffer with zeros
    sprintf(connect_msg, "CONNECT %s", client_fifo);
    if (write(server_fd, connect_msg, strlen(connect_msg)) == -1) {
        perror("write");
        close(server_fd);
        unlink(client_fifo);
        return -1;
    }
    close(server_fd);
    return 0;
}

/**
 * Creates a communication channel by creating a FIFO file.
 * 
 * @return The file descriptor of the created FIFO on success, or -1 on failure.
 */
int create_channel() {
    char client_fifo[256];
    sprintf(client_fifo, "/tmp/client_%d.fifo", getpid());
    if (mkfifo(client_fifo, 0666) == -1) {
        perror("mkfifo");
        return -1;
    }
    int client_fd = open(client_fifo, O_RDWR | O_NONBLOCK);
    if (client_fd == -1) {
        perror("open client_fifo");
        unlink(client_fifo);
        return -1;
    }
    return client_fd;
}

/**
 * Sends data through a specified FIFO file.
 * 
 * @param path The path to the FIFO file.
 * @param data The data to send.
 * @return The number of bytes written on success, or -1 on failure.
 */
ssize_t write_to_channel(const char* path, const char* data) {
    int fd = open(path, O_WRONLY);
    if (fd == -1) {
        perror("open write_to_channel");
        return -1;
    }
    ssize_t bytes_written = write(fd, data, strlen(data));
    close(fd);
    return bytes_written;
}

/**
 * Receives data from a specified file descriptor.
 * 
 * @param fd The file descriptor to read data from.
 * @param buffer The buffer to store the received data.
 * @return The number of bytes read on success, or -1 on failure.
 */
ssize_t read_from_channel(int fd, char* buffer) {
    return read(fd, buffer, 256);
}

