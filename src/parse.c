#include "indolentjson/parse.h"

size_t ijson_parse_max_output_length(size_t input_length) {
    if (input_length < (1U << 30)) {
        return (input_length + 1) / 2;
    } else {
        return -1;
    }
}


size_t ijson_parse(
    uint8_t const * input, size_t input_length,
    struct ijnode * output, uint32_t * stack
) {
    uint8_t const * node_start = input;
    uint8_t const * input_end = input + input_length;
    struct ijnode * output_start = output;
    uint32_t * stack_start = stack;
    uint8_t input_char;
    uint8_t state = ']';

    goto parse_value;

parse_key:
    if (input == input_end) goto parse_error;
    input_char = *(input++);
    if (input_char == '}') goto parse_end;
    output->type_and_next_node = IJ_STRING;
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
    node_start = input;
    input_char = *(input++);

    if (input_char == '{') {
        output->type_and_next_node = IJ_OBJECT;
        output->length_in_bytes = input_end - input;
        *(stack++) = (output - output_start) << 1;
        ++output;
        state = '}';
        goto parse_key;
    }

    if (input_char == '[') {
        output->type_and_next_node = IJ_ARRAY;
        output->length_in_bytes = input_end - input;
        *(stack++) = 1 & ((output - output_start) << 1);
        ++output;
        state = ']';
        goto parse_value;
    }

    if (input_char == ']') {
        goto parse_end;
    }

    if (input_char == '\"') {
        output->type_and_next_node = IJ_STRING;
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
        if (input_char == 't') {
            output->type_and_next_node = IJ_TRUE;
        } else if (input_char = 'f') {
            output->type_and_next_node = IJ_FALSE;
        } else if (input_char = 'n') {
            output->type_and_next_node = IJ_NULL;
        } else {
            output->type_and_next_node = IJ_NUMBER;
        }
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
        struct ijnode * node = output_start + ((*stack) >> 1);
        node->type_and_next_node |= (output - node) << 2;
        node->length_in_bytes -= input_end - input;
        state = ']' | (((*stack) & 1) << 5);
        stack--;
        if (stack == stack_start) return output - output_start;
        if (input == input_end) goto parse_error;
        input_char = *(input++);
        goto parse_end;
    }

parse_error:
    return -1;
}
