#include "indolentjson/compact.h"
#include "readfile.c"


int main(int argc, char *argv[]) {
    int input_fd = STDIN_FILENO;
    int output_fd = STDOUT_FILENO;
    uint8_t * data_buffer;
    ssize_t data_length = read_file(input_fd, &data_buffer);
    if (data_length < 0) {
        perror("Error reading file");
        return 1;
    }
    size_t output_length = ijson_compact(
        data_buffer, data_length, data_buffer
    );
    write(output_fd, data_buffer, output_length);
    uint8_t newline = '\n';
    write(output_fd, &newline, 1);
}
