/*
 * hexdump.c - Hexadecimal dump utility implementation
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
#include <ctype.h>
#include <getopt.h>
#include <sys/stat.h>

#include "hexdump.h"

static const char *program_name = "hexdump";
static int canonical_format = 0;
static int one_byte_hex = 0;
static int two_byte_decimal = 0;
static int four_byte_decimal = 0;
static size_t bytes_per_line = 16;
static off_t skip_bytes = 0;
static off_t length_limit = 0;

static void print_usage(void) {
    printf("Usage: %s [OPTIONS] [FILE...]\n\n", program_name);
    printf("Display file contents in hexadecimal format\n\n");
    printf("Options:\n");
    printf("  -C                 canonical hex+ASCII display\n");
    printf("  -x                 one-byte hex display\n");
    printf("  -d                 two-byte decimal display\n");
    printf("  -o                 two-byte octal display\n");
    printf("  -s OFFSET          skip OFFSET bytes from input\n");
    printf("  -n LENGTH          interpret only LENGTH bytes of input\n");
    printf("  -v                 display all input data (no duplicate suppression)\n");
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

static void print_hex_line(const unsigned char *data, size_t len, off_t offset) {
    size_t i;
    
    printf("%08lx  ", (unsigned long)offset);
    
    for (i = 0; i < bytes_per_line; i++) {
        if (i < len) {
            printf("%02x", data[i]);
        } else {
            printf("  ");
        }
        
        if (i % 2 == 1) printf(" ");
        if (i == 7) printf(" ");
    }
    
    printf(" |");
    for (i = 0; i < bytes_per_line && i < len; i++) {
        printf("%c", isprint(data[i]) ? data[i] : '.');
    }
    printf("|\n");
}

static void print_one_byte_hex(const unsigned char *data, size_t len, off_t offset) {
    size_t i;
    
    for (i = 0; i < len; i++) {
        if (i % 16 == 0) {
            printf("%08lx ", (unsigned long)(offset + i));
        }
        printf(" %02x", data[i]);
        if ((i + 1) % 16 == 0 || i == len - 1) {
            printf("\n");
        }
    }
}

static void print_two_byte_decimal(const unsigned char *data, size_t len, off_t offset) {
    size_t i;
    
    for (i = 0; i < len; i += 2) {
        if (i % 16 == 0) {
            printf("%08lx ", (unsigned long)(offset + i));
        }
        
        if (i + 1 < len) {
            uint16_t value = (data[i + 1] << 8) | data[i];
            printf(" %05u", value);
        } else {
            printf(" %05u", data[i]);
        }
        
        if ((i + 2) % 16 == 0 || i + 2 >= len) {
            printf("\n");
        }
    }
}

static int hexdump_file(const char *filename) {
    FILE *file;
    unsigned char buffer[4096];
    size_t bytes_read;
    off_t current_offset = 0;
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
    
    if (skip_bytes > 0) {
        if (fseeko(file, skip_bytes, SEEK_SET) != 0) {
            fprintf(stderr, "%s: %s: %s\n", program_name, filename, strerror(errno));
            result = 1;
            goto cleanup;
        }
        current_offset = skip_bytes;
    }
    
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        if (length_limit > 0 && current_offset - skip_bytes + (off_t)bytes_read > length_limit) {
            bytes_read = length_limit - (current_offset - skip_bytes);
        }
        
        if (canonical_format) {
            size_t i;
            for (i = 0; i < bytes_read; i += bytes_per_line) {
                size_t line_len = (i + bytes_per_line <= bytes_read) ? 
                                  bytes_per_line : bytes_read - i;
                print_hex_line(buffer + i, line_len, current_offset + i);
            }
        } else if (one_byte_hex) {
            print_one_byte_hex(buffer, bytes_read, current_offset);
        } else if (two_byte_decimal) {
            print_two_byte_decimal(buffer, bytes_read, current_offset);
        } else {
            size_t i;
            for (i = 0; i < bytes_read; i += bytes_per_line) {
                size_t line_len = (i + bytes_per_line <= bytes_read) ? 
                                  bytes_per_line : bytes_read - i;
                print_hex_line(buffer + i, line_len, current_offset + i);
            }
        }
        
        current_offset += bytes_read;
        
        if (length_limit > 0 && current_offset - skip_bytes >= length_limit) {
            break;
        }
    }
    
    if (ferror(file)) {
        fprintf(stderr, "%s: %s: %s\n", program_name, filename, strerror(errno));
        result = 1;
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
    
    static struct option long_options[] = {
        {"help", no_argument, 0, 'h'},
        {"version", no_argument, 0, 'V'},
        {0, 0, 0, 0}
    };
    
    while ((c = getopt_long(argc, argv, "Cxdos:n:vh", long_options, NULL)) != -1) {
        switch (c) {
            case 'C':
                canonical_format = 1;
                break;
            case 'x':
                one_byte_hex = 1;
                canonical_format = 0;
                break;
            case 'd':
                two_byte_decimal = 1;
                canonical_format = 0;
                break;
            case 'o':
                four_byte_decimal = 1;
                canonical_format = 0;
                break;
            case 's':
                skip_bytes = strtoll(optarg, NULL, 0);
                if (skip_bytes < 0) {
                    fprintf(stderr, "%s: invalid skip value\n", program_name);
                    return 1;
                }
                break;
            case 'n':
                length_limit = strtoll(optarg, NULL, 0);
                if (length_limit <= 0) {
                    fprintf(stderr, "%s: invalid length value\n", program_name);
                    return 1;
                }
                break;
            case 'v':
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
    
    if (!canonical_format && !one_byte_hex && !two_byte_decimal && !four_byte_decimal) {
        canonical_format = 1;
    }
    
    if (optind >= argc) {
        exit_code = hexdump_file(NULL);
    } else {
        for (int i = optind; i < argc; i++) {
            if (hexdump_file(argv[i]) != 0) {
                exit_code = 1;
            }
        }
    }
    
    return exit_code;
}
