#include "indolentjson/parse.h"

size_t ijson_parse_max_output_length(size_t input_length) {
    if (input_length < (1U << 30)) {
        return input_length + 1;
    } else {
        return -1;
    }
}


size_t ijson_parse(
    uint8_t const * input, size_t input_length,
    struct ijnode * output, size_t output_length,
    uint32_t * stack, size_t stack_length
) {
    uint8_t const * node_start = input;
    uint8_t const * input_end = input + input_length;
    struct ijnode * output_end = output + output_length;
    uint32_t * stack_start = stack;
    uint32_t * stack_end = stack + stack_length;

    uint8_t input_char;
    uint8_t state = ']';

    if (input == input_end) goto parse_error;
    if (((*input) & ~0x20) == '[') goto parse_value;
    goto parse_error;

parse_array:
    if (input == input_end) goto parse_error;
    input_char = *input;
    if (input_char == ']') {
        input++;
        goto parse_end;
    }
    goto parse_value;

parse_key:
    if (input == input_end) goto parse_error;
    if (output == output_end) goto parse_error;
    input_char = *(input++);
    if (input_char == '}') goto parse_end;
    node_start = input - 1;
    while (1) {
        if (input == input_end) goto parse_error;
        input_char = *(input++);
        if (input_char == '\"') {
            uint8_t const * slashes = input - 2;
            while (*(--slashes) == '\\');
            if ((input - slashes) & 1) break;
        }
    }
    output->length_in_bytes = input - node_start;
    ++output;
    if (input == input_end) goto parse_error;
    ++input; /* Skip over the ':' */

parse_value:

    if (input == input_end) goto parse_error;
    if (output == output_end) goto parse_error;
    node_start = input;
    input_char = *(input++);

    if (input_char == '{') {
        if (stack == stack_end) goto parse_error;
        output->length_in_bytes = input_end - input + 1;
        *(stack++) = 1 | ((output_end - output) << 1);
        ++output;
        state = '}';
        goto parse_key;
    }

    if (input_char == '[') {
        if (stack == stack_end) goto parse_error;
        output->length_in_bytes = input_end - input + 1;
        *(stack++) = (output_end - output) << 1;
        ++output;
        state = ']';
        goto parse_array;
    }

    if (input_char == '\"') {
        while (1) {
            if (input == input_end) goto parse_error;
            input_char = *(input++);
            if (input_char == '\"') {
                uint8_t const * slashes = input - 2;
                while (*(--slashes) == '\\');
                if ((input - slashes) & 1) break;
            }
        }
        output->length_in_bytes = input - node_start;
        ++output;
        if (input == input_end) goto parse_error;
        input_char = *(input++);
    } else {
        while(1) {
            if (input == input_end) goto parse_error;
            input_char = *(input++);
            if (input_char == ',' || input_char == state) break;
        }
        output->length_in_bytes = input - 1 - node_start;
        ++output;
    }

parse_end:
    if (input_char == ',') {
        if (state & 0x20) goto parse_key;
        else goto parse_value;
    } else {
        stack--;
        struct ijnode * node = output_end - ((*stack) >> 1);
        node->children = output - node - 1;
        node->length_in_bytes -= input_end - input;
        state = ']' | (((*stack) & 1) << 5);
        if (stack == stack_start) return 0;
        if (input == input_end) goto parse_error;
        input_char = *(input++);
        goto parse_end;
    }

parse_error:
    return -1;
}
