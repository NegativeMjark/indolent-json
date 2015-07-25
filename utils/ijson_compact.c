
#include "indolentjson/compact.h"
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

int main(int argc, char *argv[]) {
    int input_fd = STDIN_FILENO;
    int output_fd = STDOUT_FILENO;
    uint8_t * data_buffer;
    ssize_t data_length = read_file(input_fd, &data_buffer);
    if (data_length < 0) {
        perror("Error reading file");
        return -1;
    }
    size_t output_length = ijson_compact(
        data_buffer, data_length, data_buffer
    );
    write(output_fd, data_buffer, output_length);
    uint8_t newline = '\n';
    write(output_fd, &newline, 1);
}
