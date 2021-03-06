#include <stdint.h>
#include <stddef.h>

/**
 * Each node represents a JSON object, array or scalar.
 * The first node in a JSON object or array starts 1 byte after the
 * start of the object or array. Each subsequent node starts 1 byte
 * after the end of the previous node.
 * If the node is a JSON object or array then children gives the number
 * of child nodes this node has. The nodes inside the object or array
 * immediately follow this node.
 * The length_in_bytes gives the number of bytes needed to encode this
 * JSON object, array or scaler.
 */
struct ijnode {
    uint32_t children;
    uint32_t length_in_bytes;
};

/**
 * The maximum number of nodes needed to parse JSON of the given length.
 * If the input is too large to parse, i.e greater than one gigabyte, then
 * this returns (size_t)(-1). The value of (-1) is chosen such that users
 * who don't check the return value and attempt to allocate that many
 * objects will get an allocation failure.
 */
size_t ijson_parse_max_output_length(size_t input_length);

/**
 * Parse the JSON into an array of ijnodes. The input must be compact JSON
 * without any extra whitespace. The output buffer and stack should be large
 * enough to hold ijson_parse_max_output_length(input_length) entries.
 * On success returns 0. On failure returns(size_t)(-1).
 */
size_t ijson_parse(
    uint8_t const * input, size_t input_length,
    struct ijnode * output, size_t output_length,
    uint32_t * stack, size_t stack_length
);
