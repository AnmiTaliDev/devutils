/*
 * cloc.h - Count Lines of Code utility header
 *
 * Copyright (c) 2025 AnmiTaliDev
 * Created: 2025-03-20 07:40:49 UTC
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

 #ifndef CLOC_H
 #define CLOC_H
 
 #include <stdio.h>
 #include <stddef.h>
 #include <stdint.h>
 #include <sys/types.h>
 
 #ifdef __cplusplus
 extern "C" {
 #endif
 
 /* Constants */
 #define CLOC_MAX_EXT        32    /* Maximum length of file extension */
 #define CLOC_MAX_PATH      4096   /* Maximum path length */
 #define CLOC_BUF_SIZE     16384   /* Read buffer size (16KB) */
 #define CLOC_MAX_LANGUAGES  128   /* Maximum number of supported languages */
 
 /* Error codes */
 #define CLOC_SUCCESS         0    /* Operation successful */
 #define CLOC_ERROR_IO      -1    /* I/O error occurred */
 #define CLOC_ERROR_MEM     -2    /* Memory allocation failed */
 #define CLOC_ERROR_ARG     -3    /* Invalid argument */
 #define CLOC_ERROR_LIMIT   -4    /* Internal limit reached */
 
 /* Language detection flags */
 #define CLOC_LANG_NONE      0    /* Unknown language */
 #define CLOC_LANG_C         1    /* C source files */
 #define CLOC_LANG_CPP       2    /* C++ source files */
 #define CLOC_LANG_PYTHON    3    /* Python source files */
 #define CLOC_LANG_JAVA      4    /* Java source files */
 #define CLOC_LANG_GO        5    /* Go source files */
 #define CLOC_LANG_RUST      6    /* Rust source files */
 #define CLOC_LANG_JS        7    /* JavaScript source files */
 
 /* Statistics structure for a single file */
 typedef struct file_stats {
     char path[CLOC_MAX_PATH];    /* Full path to the file */
     off_t size;                  /* File size in bytes */
     uint32_t lines_code;         /* Lines of code */
     uint32_t lines_comment;      /* Comment lines */
     uint32_t lines_blank;        /* Blank lines */
     uint8_t language;            /* Language identifier */
     int error;                   /* Error code if operation failed */
 } file_stats_t;
 
 /* Language syntax definition */
 typedef struct lang_syntax {
     const char *name;            /* Language name */
     const char *extensions[8];   /* File extensions (NULL terminated) */
     const char *line_comment;    /* Single-line comment starter */
     const char *block_start;     /* Multi-line comment start */
     const char *block_end;       /* Multi-line comment end */
     uint8_t id;                 /* Language identifier */
 } lang_syntax_t;
 
 /* Aggregated statistics for a language */
 typedef struct lang_stats {
     const char *name;            /* Language name */
     uint32_t files;             /* Number of files */
     uint64_t lines_code;        /* Total lines of code */
     uint64_t lines_comment;     /* Total comment lines */
     uint64_t lines_blank;       /* Total blank lines */
     uint64_t bytes;             /* Total bytes */
 } lang_stats_t;
 
 /*
  * Initialize the CLOC system
  * Must be called before any other functions
  *
  * @return CLOC_SUCCESS on success, error code otherwise
  */
 int cloc_init(void);
 
 /*
  * Clean up CLOC system resources
  * Should be called when finished using the system
  */
 void cloc_cleanup(void);
 
 /*
  * Count lines in a single file
  *
  * @param path Path to the file
  * @param stats Pointer to stats structure to fill
  * @return CLOC_SUCCESS on success, error code otherwise
  */
 int process_file(const char *path, file_stats_t *stats);
 
 /*
  * Recursively process a directory
  *
  * @param path Directory path
  * @param callback Function to call for each file
  * @param user_data User data to pass to callback
  * @return CLOC_SUCCESS on success, error code otherwise
  */
 int process_directory(const char *path,
                      int (*callback)(const file_stats_t *, void *),
                      void *user_data);
 
 /*
  * Get language statistics
  *
  * @param stats Array to fill with language statistics
  * @param max_langs Maximum number of languages to return
  * @return Number of languages found, or -1 on error
  */
 int get_language_stats(lang_stats_t *stats, size_t max_langs);
 
 /*
  * Print statistics in standard format
  *
  * @param stats Array of language statistics
  * @param count Number of languages
  * @param stream Output stream (typically stdout)
  * @return 0 on success, -1 on error
  */
 int print_stats(const lang_stats_t *stats, size_t count, FILE *stream);
 
 /*
  * Get string representation of error code
  *
  * @param error Error code
  * @return Constant string describing the error
  */
 const char *cloc_strerror(int error);
 
 /*
  * Get supported language information
  *
  * @param syntax Array to fill with language syntax information
  * @param max_langs Maximum number of languages to return
  * @return Number of supported languages, or -1 on error
  */
 int get_supported_languages(lang_syntax_t *syntax, size_t max_langs);
 
 #ifdef __cplusplus
 }
 #endif
 
 #endif /* CLOC_H */
 
