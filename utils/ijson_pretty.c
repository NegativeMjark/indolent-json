#include "indolentjson/compact.h"
#include "indolentjson/parse.h"
#include "readfile.c"
#include <string.h>

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

static void print_pretty(
    int fd, struct ijnode * node, uint8_t * data,
    unsigned indent1, unsigned indent2, char start1, char start2
) {
    if (node->type == IJ_OBJECT) {
        print_indent(fd, indent1, start1, start2, 0);
        struct ijnode * child, * end = node + node->next_node;
        char sep = '{';
        for (child = node + 1; child != end; child += child->next_node) {
            print_pretty(fd, child, data, indent2 + 1, indent2 + 1, sep, '\n');
            child += child->next_node;
            print_pretty(fd, child, data, 0, indent2 + 1, ':', ' ');
            sep = ',';
        }
        if (sep == '{') {
            write(fd, "{}", 2);
        } else {
            print_indent(fd, indent2, '\n', 0, '}');
        }
    } else if (node->type == IJ_ARRAY) {
        print_indent(fd, indent1, start1, start2, 0);
        struct ijnode * child, * end = node + node->next_node;
        char sep = '[';
        for (child = node + 1; child != end; child += child->next_node) {
            print_pretty(fd, child, data, indent2 + 1, indent2 + 1, sep, '\n');
            sep = ',';
        }
        if (sep == '[') {
            write(fd, "[]", 2);
        } else {
            print_indent(fd, indent2, '\n', 0, ']');
        }
    } else {
        print_indent(fd, indent1, start1, start2, 0);
        write(fd, data + node->start_offset, node->end_offset - node->start_offset);
    }
}


int main(int argc, char *argv[]) {
    memset(BUFFER, '\t', sizeof(BUFFER));
    int input_fd = STDIN_FILENO;
    int output_fd = STDOUT_FILENO;
    uint8_t * data_buffer;
    ssize_t data_length = read_file(input_fd, &data_buffer);
    if (data_length < 0) {
        perror("Error reading file");
        return 1;
    }
    size_t compact_length = ijson_compact(
        data_buffer, data_length, data_buffer
    );
    size_t nodes_length = ijson_parse_max_output(compact_length);
    struct ijnode * nodes = (struct ijnode *) calloc(
        nodes_length, sizeof(struct ijnode)
    );
    if (ijson_parse(data_buffer, data_length, nodes, nodes_length) < 0) {
        perror("Error parsing JSON");
        return 1;
    }
    print_pretty(output_fd, nodes, data_buffer, 0, 0, 0, 0);
    write(output_fd, "\n", 1);
}

