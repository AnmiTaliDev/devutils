/*
 * countfile.c - File content counting utility implementation
 *
 * Copyright (c) 2025 AnmiTaliDev
 * Created: 2025-03-20 07:44:08 UTC
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
 #include <sys/stat.h>
 #include <sys/mman.h>
 
 #include "countfile.h"
 
 /* Static buffer for I/O operations */
 static char buffer[COUNTFILE_BUF_SIZE];
 
 /* Error messages */
 static const char *error_messages[] = {
     [0] = "Success",
     [-COUNTFILE_ERROR_IO] = "I/O error",
     [-COUNTFILE_ERROR_MEM] = "Memory allocation failed",
     [-COUNTFILE_ERROR_ARG] = "Invalid argument"
 };
 
 const char *count_strerror(int error) {
     error = -error;
     if (error < 0 || error >= (int)(sizeof(error_messages) / sizeof(error_messages[0]))) {
         return "Unknown error";
     }
     return error_messages[error];
 }
 
 void init_stats(file_stats_t *stats) {
     if (stats) {
         memset(stats, 0, sizeof(*stats));
         stats->filename = NULL;
         stats->size = 0;
         stats->error = COUNTFILE_SUCCESS;
     }
 }
 
 static inline int is_word_separator(char c) {
     return isspace((unsigned char)c);
 }
 
 static void count_buffer(const char *buf, size_t size, count_stats_t *stats, int *in_word) {
     for (size_t i = 0; i < size; i++) {
         stats->bytes++;
         stats->chars++;  /* For ASCII text, chars == bytes */
 
         if (buf[i] == '\n') {
             stats->lines++;
         }
 
         if (is_word_separator(buf[i])) {
             *in_word = 0;
         } else if (!*in_word) {
             *in_word = 1;
             stats->words++;
         }
     }
 }
 
 static int count_mmap(int fd, off_t size, file_stats_t *stats) {
     char *addr;
     int in_word = 0;
 
     addr = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
     if (addr == MAP_FAILED) {
         return COUNTFILE_ERROR_IO;
     }
 
     /* Advise the kernel that we'll read sequentially */
     if (posix_madvise(addr, size, POSIX_MADV_SEQUENTIAL) != 0) {
         /* Non-fatal error, continue processing */
     }
 
     count_buffer(addr, size, &stats->counts, &in_word);
     munmap(addr, size);
 
     return COUNTFILE_SUCCESS;
 }
 
 static int count_read(int fd, file_stats_t *stats) {
     ssize_t nread;
     int in_word = 0;
 
     while ((nread = read(fd, buffer, sizeof(buffer))) > 0) {
         count_buffer(buffer, nread, &stats->counts, &in_word);
     }
 
     return (nread < 0) ? COUNTFILE_ERROR_IO : COUNTFILE_SUCCESS;
 }
 
 int count_fd(int fd, file_stats_t *stats) {
     struct stat st;
 
     if (!stats) {
         return COUNTFILE_ERROR_ARG;
     }
 
     init_stats(stats);
 
     if (fstat(fd, &st) == 0) {
         stats->size = st.st_size;
     }
 
     /* Use mmap for large regular files */
     if (S_ISREG(st.st_mode) && st.st_size > COUNTFILE_BUF_SIZE) {
         return count_mmap(fd, st.st_size, stats);
     }
 
     return count_read(fd, stats);
 }
 
 int count_file(const char *filename, file_stats_t *stats) {
     int fd, result;
 
     if (!stats) {
         return COUNTFILE_ERROR_ARG;
     }
 
     init_stats(stats);
     stats->filename = filename;
 
     if (!filename) {
         return count_fd(STDIN_FILENO, stats);
     }
 
     fd = open(filename, O_RDONLY);
     if (fd == -1) {
         stats->error = COUNTFILE_ERROR_IO;
         return COUNTFILE_ERROR_IO;
     }
 
     result = count_fd(fd, stats);
     close(fd);
 
     return result;
 }
 
 int print_stats(const file_stats_t *stats, FILE *stream) {
     if (!stats || !stream) {
         return -1;
     }
 
     if (stats->filename) {
         return fprintf(stream, "%8zu %8zu %8zu %8zu %s\n",
                       stats->counts.lines,
                       stats->counts.words,
                       stats->counts.chars,
                       stats->counts.bytes,
                       stats->filename);
     } else {
         return fprintf(stream, "%8zu %8zu %8zu %8zu\n",
                       stats->counts.lines,
                       stats->counts.words,
                       stats->counts.chars,
                       stats->counts.bytes);
     }
 }
 
 #ifndef COUNTFILE_LIB_ONLY
 int main(int argc, char *argv[]) {
     file_stats_t stats, total = {0};
     int i, files = 0;
     int exit_code = 0;
 
     if (argc < 2) {
         if (count_file(NULL, &stats) != COUNTFILE_SUCCESS) {
             fprintf(stderr, "countfile: stdin: %s\n", count_strerror(stats.error));
             return 1;
         }
         print_stats(&stats, stdout);
         return 0;
     }
 
     for (i = 1; i < argc; i++) {
         if (count_file(argv[i], &stats) != COUNTFILE_SUCCESS) {
             fprintf(stderr, "countfile: %s: %s\n", 
                     argv[i], count_strerror(stats.error));
             exit_code = 1;
             continue;
         }
 
         print_stats(&stats, stdout);
         
         total.counts.lines += stats.counts.lines;
         total.counts.words += stats.counts.words;
         total.counts.chars += stats.counts.chars;
         total.counts.bytes += stats.counts.bytes;
         files++;
     }
 
     if (files > 1) {
         total.filename = "total";
         print_stats(&total, stdout);
     }
 
     return exit_code;
 }
 #endif /* COUNTFILE_LIB_ONLY */
 
