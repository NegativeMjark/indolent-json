#include "indolentjson/compact.h"
#include "indolentjson/parse.h"
#include "readfile.c"
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

static char BUFFER[4096];

static void print_indent(
    int fd, unsigned indent, char start1, char start2, char end
) {
    BUFFER[0] = BUFFER[1] = '\t';
    if (start1) {
        BUFFER[0] = start1;
        indent += 1;
        if (start2) {
            BUFFER[1] = start2;
            indent += 1;
        }
    }
    if (end) indent += 1;
    while(indent > sizeof(BUFFER)) {
        write(fd, BUFFER, sizeof(BUFFER));
        indent -= sizeof(BUFFER);
        BUFFER[0] = BUFFER[1] = '\t';
    }
    if (end) BUFFER[indent - 1] = end;
    write(fd, BUFFER, indent);
    if (end) BUFFER[indent - 1] = '\t';
}

static struct ijnode * print_pretty(
    int fd, struct ijnode * node, uint8_t * data,
    unsigned indent1, unsigned indent2, char start1, char start2
) {
    if (node->type_and_next_node & IJ_OBJECT) {
        print_indent(fd, indent1, start1, start2, 0);
        struct ijnode * child, * end = node + (node->type_and_next_node >> 2);
        char sep = '{';
        for (child = node + 1; child != end;) {
            uint8_t * child_data = data + 1;
            data = child_data + child->length_in_bytes;
            child = print_pretty(
                fd, child, child_data, indent2 + 1, indent2 + 1, sep, '\n'
            );
            child_data = data + 1;
            data = child_data + child->length_in_bytes;
            child = print_pretty(
                fd, child, child_data, 0, indent2 + 1, ':', ' '
            );
            sep = ',';
        }
        if (sep == '{') {
            write(fd, "{}", 2);
        } else {
            print_indent(fd, indent2, '\n', 0, '}');
        }
        return end;
    } else if (node->type_and_next_node & IJ_ARRAY) {
        print_indent(fd, indent1, start1, start2, 0);
        struct ijnode * child, * end = node + (node->type_and_next_node >> 2);
        char sep = '[';
        for (child = node + 1; child != end;) {
            uint8_t * child_data = data + 1;
            data = child_data + child->length_in_bytes;
            child = print_pretty(
                fd, child, child_data, indent2 + 1, indent2 + 1, sep, '\n'
            );
            sep = ',';
        }
        if (sep == '[') {
            write(fd, "[]", 2);
        } else {
            print_indent(fd, indent2, '\n', 0, ']');
        }
        return end;
    } else {
        print_indent(fd, indent1, start1, start2, 0);
        write(fd, data, node->length_in_bytes);
        return node + 1;
    }
}


int main(int argc, char *argv[]) {
    memset(BUFFER, '\t', sizeof(BUFFER));
    int input_fd = STDIN_FILENO;
    int output_fd = STDOUT_FILENO;
    if (argc > 1) {
        input_fd = open(argv[1], O_RDONLY);
    }
    uint8_t * data_buffer;
    ssize_t data_length = read_file(input_fd, &data_buffer);
    if (data_length < 0) {
        perror("Error reading file");
        return 1;
    }
    size_t compact_length = ijson_compact(
        data_buffer, data_length, data_buffer
    );
    size_t nodes_length = ijson_parse_max_output_length(compact_length);
    struct ijnode * nodes = (struct ijnode *) calloc(
        nodes_length, sizeof(struct ijnode)
    );
    uint32_t * stack = (uint32_t *) calloc(nodes_length, sizeof(uint32_t));
    if (ijson_parse(
        data_buffer, data_length, nodes, nodes_length, stack, nodes_length
    ) == (size_t)(-1)) {
        perror("Error parsing JSON");
        return 1;
    }
    print_pretty(output_fd, nodes, data_buffer, 0, 0, 0, 0);
    write(output_fd, "\n", 1);
    return 0;
}

