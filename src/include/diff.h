/*
 * diff.h - Simple file comparison utility header
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

#ifndef DIFF_H
#define DIFF_H

#include <stdio.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DIFF_VERSION "1.0.0"

typedef enum {
    DIFF_SAME = 0,
    DIFF_DIFFERENT = 1,
    DIFF_ERROR = 2
} diff_result_t;

typedef struct {
    int ignore_case;
    int ignore_whitespace;
    int brief_mode;
    int show_line_numbers;
} diff_options_t;

diff_result_t diff_files(const char *file1, const char *file2, const diff_options_t *opts);
diff_result_t diff_streams(FILE *stream1, FILE *stream2, const char *name1, const char *name2, const diff_options_t *opts);

#ifdef __cplusplus
}
#endif

#endif /* DIFF_H */
