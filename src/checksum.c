/*
 * checksum.c - File checksum calculator implementation
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

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <getopt.h>
#include <stdint.h>

#include "checksum.h"

static const char *program_name = "checksum";
static checksum_type_t checksum_type = CHECKSUM_CRC32;

static void print_usage(void) {
    printf("Usage: %s [OPTIONS] [FILE...]\n\n", program_name);
    printf("Calculate checksums for files\n\n");
    printf("Options:\n");
    printf("  -c, --crc32        calculate CRC32 checksum (default)\n");
    printf("  -s, --sum          calculate BSD sum checksum\n");
    printf("  -a, --adler32      calculate Adler-32 checksum\n");
    printf("  -v, --verify FILE  verify checksums from FILE\n");
    printf("  -q, --quiet        don't print filenames\n");
    printf("  -h, --help         display this help and exit\n");
    printf("  --version          output version information and exit\n\n");
    printf("If no FILE is specified, read from standard input.\n");
}

static void print_version(void) {
    printf("%s (dev-utils) 1.0.0\n", program_name);
    printf("Copyright (C) 2025 AnmiTaliDev\n");
    printf("License GPLv3+: GNU GPL version 3 or later <https://gnu.org/licenses/gpl.html>.\n");
    printf("This is free software: you are free to change and redistribute it.\n");
    printf("There is NO WARRANTY, to the extent permitted by law.\n");
}

static uint32_t crc32_table[256];
static int crc32_table_computed = 0;

static void make_crc32_table(void) {
    uint32_t c;
    int n, k;
    
    for (n = 0; n < 256; n++) {
        c = (uint32_t) n;
        for (k = 0; k < 8; k++) {
            if (c & 1) {
                c = 0xedb88320L ^ (c >> 1);
            } else {
                c = c >> 1;
            }
        }
        crc32_table[n] = c;
    }
    crc32_table_computed = 1;
}

static uint32_t update_crc32(uint32_t crc, const unsigned char *buf, size_t len) {
    uint32_t c = crc;
    size_t n;
    
    if (!crc32_table_computed) {
        make_crc32_table();
    }
    
    for (n = 0; n < len; n++) {
        c = crc32_table[(c ^ buf[n]) & 0xff] ^ (c >> 8);
    }
    
    return c;
}

static uint32_t crc32_checksum(const unsigned char *buf, size_t len) {
    return update_crc32(0xffffffffL, buf, len) ^ 0xffffffffL;
}

static uint32_t adler32_checksum(const unsigned char *buf, size_t len) {
    uint32_t a = 1, b = 0;
    size_t i;
    
    for (i = 0; i < len; i++) {
        a = (a + buf[i]) % 65521;
        b = (b + a) % 65521;
    }
    
    return (b << 16) | a;
}

static uint32_t bsd_sum_checksum(const unsigned char *buf, size_t len) {
    uint32_t checksum = 0;
    size_t i;
    
    for (i = 0; i < len; i++) {
        checksum = ((checksum >> 1) + ((checksum & 1) << 15) + buf[i]) & 0xffff;
    }
    
    return checksum;
}

static uint32_t calculate_checksum(const unsigned char *buf, size_t len, checksum_type_t type) {
    switch (type) {
        case CHECKSUM_CRC32:
            return crc32_checksum(buf, len);
        case CHECKSUM_ADLER32:
            return adler32_checksum(buf, len);
        case CHECKSUM_BSD_SUM:
            return bsd_sum_checksum(buf, len);
        default:
            return 0;
    }
}

static const char *get_checksum_name(checksum_type_t type) {
    switch (type) {
        case CHECKSUM_CRC32: return "CRC32";
        case CHECKSUM_ADLER32: return "ADLER32";
        case CHECKSUM_BSD_SUM: return "BSD";
        default: return "UNKNOWN";
    }
}

static int checksum_file(const char *filename, int quiet) {
    FILE *file;
    unsigned char buffer[8192];
    size_t bytes_read;
    uint32_t checksum = 0;
    uint32_t running_crc = 0xffffffffL;
    uint32_t running_adler_a = 1, running_adler_b = 0;
    uint32_t running_bsd = 0;
    int result = 0;
    
    if (!filename) {
        file = stdin;
        filename = "(standard input)";
    } else {
        file = fopen(filename, "rb");
        if (!file) {
            fprintf(stderr, "%s: %s: %s\n", program_name, filename, strerror(errno));
            return 1;
        }
    }
    
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        switch (checksum_type) {
            case CHECKSUM_CRC32:
                running_crc = update_crc32(running_crc, buffer, bytes_read);
                break;
            case CHECKSUM_ADLER32:
                for (size_t i = 0; i < bytes_read; i++) {
                    running_adler_a = (running_adler_a + buffer[i]) % 65521;
                    running_adler_b = (running_adler_b + running_adler_a) % 65521;
                }
                break;
            case CHECKSUM_BSD_SUM:
                for (size_t i = 0; i < bytes_read; i++) {
                    running_bsd = ((running_bsd >> 1) + ((running_bsd & 1) << 15) + buffer[i]) & 0xffff;
                }
                break;
        }
    }
    
    switch (checksum_type) {
        case CHECKSUM_CRC32:
            checksum = running_crc ^ 0xffffffffL;
            break;
        case CHECKSUM_ADLER32:
            checksum = (running_adler_b << 16) | running_adler_a;
            break;
        case CHECKSUM_BSD_SUM:
            checksum = running_bsd;
            break;
    }
    
    if (ferror(file)) {
        fprintf(stderr, "%s: %s: %s\n", program_name, filename, strerror(errno));
        result = 1;
        goto cleanup;
    }
    
    if (quiet) {
        printf("%08x\n", checksum);
    } else {
        printf("%08x  %s\n", checksum, filename);
    }
    
cleanup:
    if (file != stdin) {
        fclose(file);
    }
    
    return result;
}

int main(int argc, char *argv[]) {
    int c;
    int exit_code = 0;
    int quiet = 0;
    const char *verify_file = NULL;
    
    static struct option long_options[] = {
        {"crc32", no_argument, 0, 'c'},
        {"sum", no_argument, 0, 's'},
        {"adler32", no_argument, 0, 'a'},
        {"verify", required_argument, 0, 'v'},
        {"quiet", no_argument, 0, 'q'},
        {"help", no_argument, 0, 'h'},
        {"version", no_argument, 0, 'V'},
        {0, 0, 0, 0}
    };
    
    while ((c = getopt_long(argc, argv, "csav:qh", long_options, NULL)) != -1) {
        switch (c) {
            case 'c':
                checksum_type = CHECKSUM_CRC32;
                break;
            case 's':
                checksum_type = CHECKSUM_BSD_SUM;
                break;
            case 'a':
                checksum_type = CHECKSUM_ADLER32;
                break;
            case 'v':
                verify_file = optarg;
                break;
            case 'q':
                quiet = 1;
                break;
            case 'h':
                print_usage();
                return 0;
            case 'V':
                print_version();
                return 0;
            case '?':
            default:
                fprintf(stderr, "Try '%s --help' for more information.\n", program_name);
                return 1;
        }
    }
    
    if (verify_file) {
        fprintf(stderr, "%s: verify functionality not yet implemented\n", program_name);
        return 1;
    }
    
    if (optind >= argc) {
        exit_code = checksum_file(NULL, quiet);
    } else {
        for (int i = optind; i < argc; i++) {
            if (checksum_file(argv[i], quiet) != 0) {
                exit_code = 1;
            }
        }
    }
    
    return exit_code;
}
