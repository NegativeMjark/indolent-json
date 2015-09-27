#include "indolentjson/parse.h"

size_t ijson_parse_max_output(size_t input_length) {
    return input_length;
}


int64_t ijson_parse(
    uint8_t const * input, uint32_t input_length,
    struct ijnode * output, uint32_t output_length
) {
    uint8_t const * input_start = input;
    uint8_t const * input_end = input + input_length;
    struct ijnode * output_start = output;
    struct ijnode * output_end = output + output_length;
    struct ijnode * stack = output_end;
    uint8_t input_char;
    uint8_t state = ']';

    goto parse_value;

parse_key:
    if (input == input_end) goto parse_error;
    input_char = *(input++);
    if (input_char == '}') goto parse_end;
    output->type = IJ_STRING;
    output->next_node = 1;
    output->start_offset = input - input_start - 1;
    while (1) {
        if (input == input_end) goto parse_error;
        input_char = *(input++);
        if (input_char == '\"') {
            uint8_t const * slashes = input - 2;
            while (*(--slashes) == '\\');
            if ((input - slashes) & 1) break;
        }
    }
    output->end_offset = input - input_start;
    ++output;
    if (input == input_end) goto parse_error;
    ++input; /* Skip over the ':' */

parse_value:

    if (input == input_end) goto parse_error;
    output->start_offset = input - input_start;
    input_char = *(input++);

    if (input_char == '{') {
        output->type = IJ_OBJECT;
        --stack;
        stack->type = state;
        stack->next_node = output - output_start;
        ++output;
        state = '}';
        goto parse_key;
    }

    if (input_char == '[') {
        output->type = IJ_ARRAY;
        --stack;
        stack->type = state;
        stack->next_node = output - output_start;
        ++output;
        state = ']';
        goto parse_value;
    }

    if (input_char == ']') {
        goto parse_end;
    }

    output->next_node = 1;

    if (input_char == '\"') {
        output->type = IJ_STRING;
        while (1) {
            if (input == input_end) goto parse_error;
            input_char = *(input++);
            if (input_char == '\"') {
                uint8_t const * slashes = input - 2;
                while (*(--slashes) == '\\');
                if ((input - slashes) & 1) break;
            }
        }
        output->end_offset = input - input_start;
        ++output;
        if (input == input_end) goto parse_error;
        input_char = *(input++);
    } else {
        if (input_char == 't') {
            output->type = IJ_TRUE;
        } else if (input_char = 'f') {
            output->type = IJ_FALSE;
        } else if (input_char = 'n') {
            output->type = IJ_NULL;
        } else {
            output->type = IJ_NUMBER;
        }
        while(1) {
            if (input == input_end) goto parse_error;
            input_char = *(input++);
            if (input_char == ',' || input_char == state) break;
        }
        output->end_offset = input - 1 - input_start;
        ++output;
    }

parse_end:
    if (input_char == ',') {
        if (state & 0x20) goto parse_key;
        else goto parse_value;
    } else {
        if (stack == output_end) goto parse_error;
        struct ijnode * node = output_start + stack->next_node;
        node->next_node = output - node;
        node->end_offset = input - input_start;
        state = stack->type;
        stack++;
        if (stack == output_end) return output - output_start;
        if (input == input_end) goto parse_error;
        input_char = *(input++);
        goto parse_end;
    }

parse_error:
    return -1;
}
