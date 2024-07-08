#include "kv_lib.h"
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

/**
 * Establishes a connection to the server.
 * 
 * @param server_fifo The path to the server FIFO (named pipe).
 * @param client_fifo The buffer to store the path to the client FIFO (named pipe).
 * @return 0 on success, or -1 on failure.
 * 
 * This function creates a unique client FIFO based on the process ID, opens the server FIFO,
 * sends a connect message to the server with the path to the client FIFO, and then closes the server FIFO.
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
 * Sends data through a specified file descriptor.
 * 
 * @param fd The file descriptor to send data through.
 * @param data The data to send.
 * @return The number of bytes written on success, or -1 on failure.
 * 
 * This function writes the given data to the provided file descriptor.
 */
int send_data(int fd, const char* data) {
    return write(fd, data, strlen(data));
}

/**
 * Receives data from a specified file descriptor.
 * 
 * @param fd The file descriptor to read data from.
 * @param buffer The buffer to store the received data.
 * @param size The size of the buffer.
 * @return The number of bytes read on success, or -1 on failure.
 * 
 * This function reads data from the provided file descriptor into the buffer up to the specified size.
 */
int receive_data(int fd, char* buffer, size_t size) {
    return read(fd, buffer, size);
}

/**
 * Terminates the connection by closing the file descriptor.
 * 
 * @param fd The file descriptor to close.
 * @return 0 on success, or -1 on failure.
 * 
 * This function closes the provided file descriptor.
 */
int terminate_connection(int fd) {
    return close(fd);
}
