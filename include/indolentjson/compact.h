/* Copyright 2015 Mark Haines
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

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
