#include <stdint.h>
#include <stddef.h>

/**
 * Reduce JSON to its most compact UTF-8 representation.
 * The output buffer must have at least the capacity of the
 * input buffer. The output buffer may be the same as the input.
 * Returns the size of the compact JSON.
 */
size_t ijson_compact(
    uint8_t const * input, size_t input_length,
    uint8_t * output
);
