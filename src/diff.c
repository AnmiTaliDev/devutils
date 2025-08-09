/*
 * diff.c - Simple file comparison utility implementation
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
#include <getopt.h>

#include "diff.h"

static const char *program_name = "diff";
static int ignore_case = 0;
static int ignore_whitespace = 0;
static int brief_mode = 0;

static void print_usage(void) {
    printf("Usage: %s [OPTIONS] FILE1 FILE2\n\n", program_name);
    printf("Compare files line by line\n\n");
    printf("Options:\n");
    printf("  -i, --ignore-case     ignore case differences\n");
    printf("  -w, --ignore-all-space ignore all white space\n");
    printf("  -q, --brief           report only when files differ\n");
    printf("  -h, --help            display this help and exit\n");
    printf("  --version             output version information and exit\n");
}

static void print_version(void) {
    printf("%s (dev-utils) 1.0.0\n", program_name);
    printf("Copyright (C) 2025 AnmiTaliDev\n");
    printf("License GPLv3+: GNU GPL version 3 or later <https://gnu.org/licenses/gpl.html>.\n");
    printf("This is free software: you are free to change and redistribute it.\n");
    printf("There is NO WARRANTY, to the extent permitted by law.\n");
}

static char *normalize_line(const char *line) {
    if (!line) return NULL;
    
    size_t len = strlen(line);
    char *normalized = malloc(len + 1);
    if (!normalized) return NULL;
    
    char *dest = normalized;
    const char *src = line;
    
    while (*src) {
        if (ignore_whitespace && (*src == ' ' || *src == '\t')) {
            src++;
            continue;
        }
        
        if (ignore_case) {
            *dest = (*src >= 'A' && *src <= 'Z') ? *src + 32 : *src;
        } else {
            *dest = *src;
        }
        
        dest++;
        src++;
    }
    *dest = '\0';
    
    return normalized;
}

static int compare_files(const char *file1, const char *file2) {
    FILE *f1, *f2;
    char *line1 = NULL, *line2 = NULL;
    size_t len1 = 0, len2 = 0;
    ssize_t read1, read2;
    int line_num = 1;
    int differences = 0;
    int result = 0;
    
    f1 = fopen(file1, "r");
    if (!f1) {
        fprintf(stderr, "%s: %s: %s\n", program_name, file1, strerror(errno));
        return 2;
    }
    
    f2 = fopen(file2, "r");
    if (!f2) {
        fprintf(stderr, "%s: %s: %s\n", program_name, file2, strerror(errno));
        fclose(f1);
        return 2;
    }
    
    while (1) {
        read1 = getline(&line1, &len1, f1);
        read2 = getline(&line2, &len2, f2);
        
        if (read1 == -1 && read2 == -1) {
            break;
        }
        
        if (read1 == -1) {
            if (!brief_mode) {
                printf("%da%d\n", line_num - 1, line_num);
                printf("> %s", line2);
            }
            differences = 1;
            line_num++;
            continue;
        }
        
        if (read2 == -1) {
            if (!brief_mode) {
                printf("%dd%d\n", line_num, line_num - 1);
                printf("< %s", line1);
            }
            differences = 1;
            line_num++;
            continue;
        }
        
        char *norm1 = normalize_line(line1);
        char *norm2 = normalize_line(line2);
        
        if (!norm1 || !norm2) {
            fprintf(stderr, "%s: memory allocation failed\n", program_name);
            result = 2;
            free(norm1);
            free(norm2);
            break;
        }
        
        if (strcmp(norm1, norm2) != 0) {
            if (!brief_mode) {
                printf("%dc%d\n", line_num, line_num);
                printf("< %s", line1);
                printf("---\n");
                printf("> %s", line2);
            }
            differences = 1;
        }
        
        free(norm1);
        free(norm2);
        line_num++;
    }
    
    if (brief_mode && differences) {
        printf("Files %s and %s differ\n", file1, file2);
    }
    
    free(line1);
    free(line2);
    fclose(f1);
    fclose(f2);
    
    return differences ? 1 : 0;
}

int main(int argc, char *argv[]) {
    int c;
    
    static struct option long_options[] = {
        {"ignore-case", no_argument, 0, 'i'},
        {"ignore-all-space", no_argument, 0, 'w'},
        {"brief", no_argument, 0, 'q'},
        {"help", no_argument, 0, 'h'},
        {"version", no_argument, 0, 'V'},
        {0, 0, 0, 0}
    };
    
    while ((c = getopt_long(argc, argv, "iwqh", long_options, NULL)) != -1) {
        switch (c) {
            case 'i':
                ignore_case = 1;
                break;
            case 'w':
                ignore_whitespace = 1;
                break;
            case 'q':
                brief_mode = 1;
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
                return 2;
        }
    }
    
    if (argc - optind != 2) {
        fprintf(stderr, "%s: missing operand\n", program_name);
        fprintf(stderr, "Try '%s --help' for more information.\n", program_name);
        return 2;
    }
    
    return compare_files(argv[optind], argv[optind + 1]);
}
