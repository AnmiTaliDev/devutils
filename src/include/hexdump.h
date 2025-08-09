/*
 * hexdump.h - Hexadecimal dump utility header
 *
 * Copyright (c) 2025 AnmiTaliDev
 * Created: 2025-08-09
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef HEXDUMP_H
#define HEXDUMP_H

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define HEXDUMP_VERSION "1.0.0"

typedef enum {
    HEXDUMP_FORMAT_CANONICAL,
    HEXDUMP_FORMAT_ONE_BYTE_HEX,
    HEXDUMP_FORMAT_TWO_BYTE_DECIMAL,
    HEXDUMP_FORMAT_TWO_BYTE_OCTAL
} hexdump_format_t;

typedef struct {
    hexdump_format_t format;
    size_t bytes_per_line;
    off_t skip_bytes;
    off_t length_limit;
    int suppress_duplicates;
} hexdump_options_t;

int hexdump_file_with_options(const char *filename, const hexdump_options_t *opts);

#ifdef __cplusplus
}
#endif

#endif /* HEXDUMP_H */
