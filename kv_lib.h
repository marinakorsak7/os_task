#ifndef KV_LIB_H
#define KV_LIB_H

#include <fcntl.h>
#include <unistd.h>

typedef enum {
    DataType_String,
    DataType_Int,
    DataType_Long
} DataType;

int create_channel(void);
ssize_t write_to_channel(const char* path, const char* data);
ssize_t read_from_channel(int fd, char* buffer);

#endif /* KV_LIB_H */

