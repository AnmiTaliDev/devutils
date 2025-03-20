/*
 * countfile.h - File content counting utility header
 *
 * Copyright (c) 2025 AnmiTaliDev
 * Created: 2025-03-20 07:36:28 UTC
 *
 * This file is part of dev-utils and is released under the MIT License.
 * See the LICENSE file for more details.
 */

 #ifndef COUNTFILE_H
 #define COUNTFILE_H
 
 #include <stdio.h>
 #include <stddef.h>
 #include <sys/types.h>
 
 #ifdef __cplusplus
 extern "C" {
 #endif
 
 /* Size for internal buffers */
 #define COUNTFILE_BUF_SIZE (16 * 1024)  /* 16KB buffer */
 
 /* Error codes */
 #define COUNTFILE_SUCCESS       0   /* Operation successful */
 #define COUNTFILE_ERROR_IO    -1   /* I/O error occurred */
 #define COUNTFILE_ERROR_MEM   -2   /* Memory allocation failed */
 #define COUNTFILE_ERROR_ARG   -3   /* Invalid argument */
 
 /* Main count structure */
 typedef struct count_stats {
     size_t lines;     /* Number of newlines */
     size_t words;     /* Number of words (whitespace-separated) */
     size_t chars;     /* Number of characters */
     size_t bytes;     /* Number of bytes */
 } count_stats_t;
 
 /* File statistics structure */
 typedef struct file_stats {
     const char *filename;  /* Name of the file (can be NULL for stdin) */
     off_t size;           /* File size in bytes */
     int error;            /* Error code if operation failed */
     count_stats_t counts; /* Counting results */
 } file_stats_t;
 
 /*
  * Count contents of a file
  * 
  * @param filename Path to the file to count (NULL for stdin)
  * @param stats Pointer to stats structure to fill
  * @return COUNTFILE_SUCCESS on success, error code otherwise
  */
 int count_file(const char *filename, file_stats_t *stats);
 
 /*
  * Count contents from a file descriptor
  * 
  * @param fd File descriptor to read from
  * @param stats Pointer to stats structure to fill
  * @return COUNTFILE_SUCCESS on success, error code otherwise
  */
 int count_fd(int fd, file_stats_t *stats);
 
 /*
  * Print counting results in standard format
  * 
  * @param stats Pointer to stats structure to print
  * @param stream Output stream (typically stdout)
  * @return 0 on success, -1 on error
  */
 int print_stats(const file_stats_t *stats, FILE *stream);
 
 /*
  * Initialize stats structure with default values
  * 
  * @param stats Pointer to stats structure to initialize
  */
 void init_stats(file_stats_t *stats);
 
 /*
  * Get string representation of error code
  * 
  * @param error Error code from count operations
  * @return Constant string describing the error
  */
 const char *count_strerror(int error);
 
 #ifdef __cplusplus
 }
 #endif
 
 #endif /* COUNTFILE_H */