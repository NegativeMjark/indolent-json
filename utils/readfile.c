#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stddef.h>

ssize_t read_file(
    int fd,
    uint8_t **buffer
) {
    size_t buffer_size = 4096;
    uint8_t * current_buffer = (uint8_t *) malloc(buffer_size);
    if (current_buffer == NULL) return -1;
    size_t buffer_pos = 0;
    while (1) {
        ssize_t count = read(
            fd, current_buffer + buffer_pos, buffer_size - buffer_pos
        );
        if (count < 0) break;
        if (count == 0) {
            uint8_t * return_buffer = realloc(current_buffer, buffer_pos);
            if (return_buffer == NULL) break;
            *buffer = return_buffer;
            return buffer_pos;
        }
        buffer_pos += count;
        if (buffer_pos == buffer_size) {
            buffer_size *= 2;
            uint8_t * new_buffer = realloc(current_buffer, buffer_size);
            if (new_buffer == NULL) break;
            current_buffer = new_buffer;
        }
    }
    free(current_buffer);
    return -1;
}
