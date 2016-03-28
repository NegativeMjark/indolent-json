/* Copyright 2015 Mark Haines
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "indolentjson/compact.h"


static uint32_t read_hexdigit_4(
    uint8_t const * input
) {
    unsigned hex = *input;
    hex <<= 8; hex |= *(++input);
    hex <<= 8; hex |= *(++input);
    hex <<= 8; hex |= *(++input);
    hex -= 0x30303030;
    hex &= 0x1F1F1F1F;
    unsigned mask = hex & 0x10101010;
    hex += mask >> 4;
    hex -= mask >> 1;
    hex |= hex >> 4;
    hex &= 0xFF00FF;
    hex |= hex >> 8;
    return hex & 0xFFFF;
}


size_t ijson_compact(
    uint8_t const * input, size_t input_length,
    uint8_t * output
) {
    uint8_t * output_start = output;
    uint8_t const * input_end = input + input_length;
    unsigned input_char;

compact_token:
    if (input == input_end) return output - output_start;
    input_char = *(input++);
    while (input_char < '!') {
        if (input == input_end) return output - output_start;
        input_char = *(input++);
    }
    *(output++) = input_char;
    if (input_char == '\"') goto compact_string;
    goto compact_token;

compact_string:
    if (input == input_end) return 0;
    input_char = *(input++);
    if (input_char == '\\') {
        if (input == input_end) return 0;
        input_char = *(input++);
        if (input_char == 'u') {
            if (input_end - input < 4) return 0;
            input_char = read_hexdigit_4(input);
            input += 4;
            if (input_char == '\"' || input_char == '\\') {}
            else if (input_char < '\"') {
                if (input_char == '\b') { input_char = 'b'; }
                else if (input_char == '\f') { input_char = 'f'; }
                else if (input_char == '\n') { input_char = 'n'; }
                else if (input_char == '\r') { input_char = 'r'; }
                else if (input_char == '\t') { input_char = 't'; }
                else {
                    *(output++) = '\\';
                    *(output++) = 'u';
                    *(output++) = '0';
                    *(output++) = '0';
                    *(output++) = '0' + (input_char >> 4);
                    *(output++) = "0123456789ABCDEF"[input_char & 0xF];
                    goto compact_string;
                }
            } else if (input_char < 0x80) {
                goto compact_string_output0;
            } else if (input_char < 0x800) {
                *(output++) = 0xC0 | (input_char >> 6);
                goto compact_string_output1;
            } else {
                if (input_char < 0xD800 || input_char >= 0xE000) {
                    *(output++) = 0xE0 | (input_char >> 12);
                    goto compact_string_output2;
                } else {
                    if (input_end - input < 6) return 0;
                    input_char &= 0x3FF;
                    input_char <<= 10;
                    input_char |= 0x3FF & read_hexdigit_4(input + 2);
                    input_char += 0x10000;
                    input += 6;
                    *(output++) = 0xF0 | (input_char >> 18);
                    *(output++) = 0x80 | ((input_char >> 12) & 0x3F);
                    goto compact_string_output2;
                }
            }
        }
        if (input_char == '/') {} else *(output++) = '\\';
    } else if (input_char == '\"') {
        *(output++) = '\"';
        goto compact_token;
    }
    goto compact_string_output0;
compact_string_output2:
    *(output++) = 0x80 | ((input_char >> 6) & 0x3F);
compact_string_output1:
    input_char = 0x80 | (input_char & 0x3F);
compact_string_output0:
    *(output++) = input_char;
    goto compact_string;
}
