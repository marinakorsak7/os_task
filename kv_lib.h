#ifndef KV_LIB_H
#define KV_LIB_H

#include <stdio.h>

// Function declarations

/**
 * Establishes a connection to the server.
 * 
 * @param server_fifo The path to the server FIFO (named pipe).
 * @param client_fifo The buffer to store the path to the client FIFO (named pipe).
 * @return The file descriptor for the client FIFO on success, or -1 on failure.
 */
int establish_connection(const char* server_fifo, char* client_fifo);

/**
 * Sends data through a specified file descriptor.
 * 
 * @param fd The file descriptor to send data through.
 * @param data The data to send.
 * @return The number of bytes written on success, or -1 on failure.
 */
int send_data(int fd, const char* data);

/**
 * Receives data from a specified file descriptor.
 * 
 * @param fd The file descriptor to read data from.
 * @param buffer The buffer to store the received data.
 * @param size The size of the buffer.
 * @return The number of bytes read on success, or -1 on failure.
 */
int receive_data(int fd, char* buffer, size_t size);

/**
 * Terminates the connection by closing the file descriptor.
 * 
 * @param fd The file descriptor to close.
 * @return 0 on success, or -1 on failure.
 */
int terminate_connection(int fd);

#endif // KV_LIB_H
