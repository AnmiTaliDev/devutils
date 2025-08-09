/*
 * checksum.h - File checksum calculator header
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

#ifndef CHECKSUM_H
#define CHECKSUM_H

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CHECKSUM_VERSION "1.0.0"

typedef enum {
    CHECKSUM_CRC32,
    CHECKSUM_ADLER32,
    CHECKSUM_BSD_SUM
} checksum_type_t;

typedef struct {
    uint32_t value;
    checksum_type_t type;
    size_t bytes_processed;
} checksum_result_t;

uint32_t checksum_crc32(const unsigned char *data, size_t len);
uint32_t checksum_adler32(const unsigned char *data, size_t len);
uint32_t checksum_bsd_sum(const unsigned char *data, size_t len);

int checksum_file_stream(FILE *stream, checksum_type_t type, checksum_result_t *result);
int checksum_verify_file(const char *checksum_file);

#ifdef __cplusplus
}
#endif

#endif /* CHECKSUM_H */
