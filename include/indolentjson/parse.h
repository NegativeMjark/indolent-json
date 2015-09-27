#include <stdint.h>
#include <stddef.h>

static const uint32_t IJ_ARRAY  = 0x03;
static const uint32_t IJ_OBJECT = 0x05;
static const uint32_t IJ_STRING = 0x08;
static const uint32_t IJ_NUMBER = 0x10;
static const uint32_t IJ_TRUE   = 0x20;
static const uint32_t IJ_FALSE  = 0x40;
static const uint32_t IJ_NULL   = 0x80;

struct ijnode {
    uint32_t type;
    uint32_t next_node;
    uint32_t start_offset;
    uint32_t end_offset;
};

size_t ijson_parse_max_output(size_t input_length);

int64_t ijson_parse(
    uint8_t const * input, uint32_t input_length,
    struct ijnode * output, uint32_t ouput_length
);
