/*
 * cloc.c - Count Lines of Code utility implementation
 *
 * Copyright (c) 2025 AnmiTaliDev
 * Created: 2025-03-20 08:19:41 UTC
 * Author: AnmiTaliDev
 *
 * This file is part of dev-utils and is released under the MIT License.
 * See the LICENSE file for more details.
 */

 #define _POSIX_C_SOURCE 200809L

 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <errno.h>
 #include <fcntl.h>
 #include <unistd.h>
 #include <ctype.h>
 #include <dirent.h>
 #include <sys/stat.h>
 #include <sys/mman.h>
 
 #include "cloc.h"
 
 /* Maximum length for examining file content for language detection */
 #define MAX_EXAMINE_SIZE 4096
 
 /* Language definitions */
 static const lang_syntax_t languages[] = {
     {"C",          {".c", ".h", NULL},                "//", "/*", "*/",    CLOC_LANG_C},
     {"C++",        {".cpp", ".hpp", ".cc", ".hxx", ".cxx", NULL}, 
                                                       "//", "/*", "*/",    CLOC_LANG_CPP},
     {"Python",     {".py", ".pyw", NULL},             "#",  "\"\"\"", "\"\"\"", CLOC_LANG_PYTHON},
     {"Java",       {".java", NULL},                   "//", "/*", "*/",    CLOC_LANG_JAVA},
     {"Go",         {".go", NULL},                     "//", "/*", "*/",    CLOC_LANG_GO},
     {"Rust",       {".rs", NULL},                     "//", "/*", "*/",    CLOC_LANG_RUST},
     {"JavaScript", {".js", ".jsx", ".mjs", NULL},     "//", "/*", "*/",    CLOC_LANG_JS},
     {NULL,         {NULL},                            NULL, NULL,  NULL,    CLOC_LANG_NONE}
 };
 
 /* Global statistics storage */
 static lang_stats_t *global_stats = NULL;
 static size_t global_stats_count = 0;
 
 /* C++ keywords for language detection */
 static const char *cpp_keywords[] = {
     "class", "namespace", "template", "typename", "operator",
     "virtual", "public:", "private:", "protected:", "friend",
     NULL
 };
 
 static void init_file_stats(file_stats_t *stats) {
     if (stats) {
         memset(stats, 0, sizeof(*stats));
     }
 }
 
 static int is_cpp_file(const char *content, size_t size) {
     char *buf = malloc(size + 1);
     if (!buf) return 0;
     
     memcpy(buf, content, size);
     buf[size] = '\0';
 
     int is_cpp = 0;
     for (const char **keyword = cpp_keywords; *keyword; keyword++) {
         if (strstr(buf, *keyword)) {
             is_cpp = 1;
             break;
         }
     }
 
     free(buf);
     return is_cpp;
 }
 
 static const lang_syntax_t *detect_language(const char *path) {
     const char *ext = strrchr(path, '.');
     if (!ext) return NULL;
 
     /* First check file extension */
     for (const lang_syntax_t *lang = languages; lang->name; lang++) {
         for (size_t i = 0; lang->extensions[i]; i++) {
             if (strcmp(ext, lang->extensions[i]) == 0) {
                 /* Special handling for .h files */
                 if (strcmp(ext, ".h") == 0) {
                     FILE *f = fopen(path, "rb");
                     if (!f) return &languages[0]; /* Default to C */
                     
                     char buf[MAX_EXAMINE_SIZE];
                     size_t n = fread(buf, 1, sizeof(buf), f);
                     fclose(f);
 
                     return is_cpp_file(buf, n) ? &languages[1] : &languages[0];
                 }
                 return lang;
             }
         }
     }
     return NULL;
 }
 
 static void count_lines(const char *buf, size_t size, const lang_syntax_t *lang, file_stats_t *stats) {
     const char *p = buf;
     const char *end = buf + size;
     int in_block_comment = 0;
     int in_line_comment = 0;
     int has_code = 0;
     int in_string = 0;
     char string_char = 0;
 
     stats->size = size;
 
     while (p < end) {
         /* Handle end of line */
         if (*p == '\n' || p == end - 1) {
             if (in_line_comment) {
                 stats->lines_comment++;
             } else if (in_block_comment) {
                 stats->lines_comment++;
             } else if (has_code) {
                 stats->lines_code++;
             } else {
                 stats->lines_blank++;
             }
 
             in_line_comment = 0;
             has_code = 0;
             p++;
             continue;
         }
 
         /* Skip whitespace */
         if (isspace((unsigned char)*p)) {
             p++;
             continue;
         }
 
         /* Handle strings */
         if (!in_line_comment && !in_block_comment) {
             if (!in_string && (*p == '"' || *p == '\'')) {
                 in_string = 1;
                 string_char = *p;
                 has_code = 1;
             } else if (in_string && *p == string_char && *(p-1) != '\\') {
                 in_string = 0;
             }
         }
 
         if (in_string) {
             p++;
             continue;
         }
 
         /* Handle block comments */
         if (!in_line_comment && !in_block_comment && 
             p + strlen(lang->block_start) <= end &&
             memcmp(p, lang->block_start, strlen(lang->block_start)) == 0) {
             in_block_comment = 1;
             p += strlen(lang->block_start);
             continue;
         }
 
         if (in_block_comment && 
             p + strlen(lang->block_end) <= end &&
             memcmp(p, lang->block_end, strlen(lang->block_end)) == 0) {
             in_block_comment = 0;
             p += strlen(lang->block_end);
             continue;
         }
 
         /* Handle line comments */
         if (!in_block_comment && !in_line_comment &&
             p + strlen(lang->line_comment) <= end &&
             memcmp(p, lang->line_comment, strlen(lang->line_comment)) == 0) {
             in_line_comment = 1;
             p += strlen(lang->line_comment);
             continue;
         }
 
         /* Count code */
         if (!in_block_comment && !in_line_comment) {
             has_code = 1;
         }
 
         p++;
     }
 
     /* Handle last line if it doesn't end with newline */
     if (has_code && *(end-1) != '\n') {
         stats->lines_code++;
     }
 }
 
 static int process_file(const char *path, file_stats_t *stats) {
     int fd, result = CLOC_SUCCESS;
     struct stat st;
     char *addr;
     const lang_syntax_t *lang;
 
     init_file_stats(stats);
     strncpy(stats->path, path, CLOC_MAX_PATH - 1);
     stats->path[CLOC_MAX_PATH - 1] = '\0';
 
     lang = detect_language(path);
     if (!lang) {
         return CLOC_ERROR_ARG;
     }
 
     stats->language = lang->id;
 
     fd = open(path, O_RDONLY);
     if (fd == -1) {
         stats->error = CLOC_ERROR_IO;
         return CLOC_ERROR_IO;
     }
 
     if (fstat(fd, &st) == -1) {
         close(fd);
         return CLOC_ERROR_IO;
     }
 
     if (st.st_size == 0) {
         close(fd);
         return CLOC_SUCCESS;
     }
 
     addr = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
     if (addr == MAP_FAILED) {
         close(fd);
         return CLOC_ERROR_IO;
     }
 
     if (posix_madvise(addr, st.st_size, POSIX_MADV_SEQUENTIAL) != 0) {
         /* Non-fatal error, continue processing */
     }
 
     count_lines(addr, st.st_size, lang, stats);
 
     munmap(addr, st.st_size);
     close(fd);
 
     return result;
 }
 
 int process_directory(const char *path,
                      int (*callback)(const file_stats_t *, void *),
                      void *user_data) {
     DIR *dir;
     struct dirent *entry;
     char full_path[CLOC_MAX_PATH];
     struct stat st;
     file_stats_t stats;
     int result = CLOC_SUCCESS;
 
     dir = opendir(path);
     if (!dir) {
         return CLOC_ERROR_IO;
     }
 
     while ((entry = readdir(dir))) {
         /* Skip hidden files and special directories */
         if (entry->d_name[0] == '.' || 
             strcmp(entry->d_name, "..") == 0) {
             continue;
         }
 
         if (strlen(path) + strlen(entry->d_name) + 2 > CLOC_MAX_PATH) {
             continue;
         }
 
         snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);
 
         if (lstat(full_path, &st) == -1) {
             continue;
         }
 
         if (S_ISDIR(st.st_mode)) {
             result = process_directory(full_path, callback, user_data);
             if (result != CLOC_SUCCESS) break;
         } else if (S_ISREG(st.st_mode)) {
             if (process_file(full_path, &stats) == CLOC_SUCCESS) {
                 if (callback && callback(&stats, user_data) != 0) {
                     result = CLOC_ERROR_ARG;
                     break;
                 }
             }
         }
     }
 
     closedir(dir);
     return result;
 }
 
 static int update_stats_callback(const file_stats_t *file_stats, void *user_data) {
     (void)user_data;  /* Unused */
     lang_stats_t *lang_stats = NULL;
 
     /* Find or create language stats */
     for (size_t i = 0; i < global_stats_count; i++) {
         if (global_stats[i].name == languages[file_stats->language].name) {
             lang_stats = &global_stats[i];
             break;
         }
     }
 
     if (!lang_stats) {
         if (global_stats_count >= CLOC_MAX_LANGUAGES) {
             return CLOC_ERROR_LIMIT;
         }
         lang_stats = &global_stats[global_stats_count++];
         lang_stats->name = languages[file_stats->language].name;
     }
 
     /* Update statistics */
     lang_stats->files++;
     lang_stats->lines_code += file_stats->lines_code;
     lang_stats->lines_comment += file_stats->lines_comment;
     lang_stats->lines_blank += file_stats->lines_blank;
     lang_stats->bytes += file_stats->size;
 
     return CLOC_SUCCESS;
 }
 
 int cloc_init(void) {
     global_stats = calloc(CLOC_MAX_LANGUAGES, sizeof(lang_stats_t));
     if (!global_stats) {
         return CLOC_ERROR_MEM;
     }
     global_stats_count = 0;
     return CLOC_SUCCESS;
 }
 
 void cloc_cleanup(void) {
     free(global_stats);
     global_stats = NULL;
     global_stats_count = 0;
 }
 
 int print_stats(const lang_stats_t *stats, size_t count, FILE *stream) {
     uint64_t total_files = 0, total_code = 0;
     uint64_t total_comment = 0, total_blank = 0;
 
     if (!stats || !stream) return -1;
 
     fprintf(stream, "\nLanguage     Files     Code  Comments    Blank    Total\n");
     fprintf(stream, "------------------------------------------------------\n");
 
     for (size_t i = 0; i < count; i++) {
         fprintf(stream, "%-10s %7u %8u %9u %8u %8u\n",
                 stats[i].name,
                 (unsigned)stats[i].files,
                 (unsigned)stats[i].lines_code,
                 (unsigned)stats[i].lines_comment,
                 (unsigned)stats[i].lines_blank,
                 (unsigned)(stats[i].lines_code + 
                           stats[i].lines_comment + 
                           stats[i].lines_blank));
 
         total_files += stats[i].files;
         total_code += stats[i].lines_code;
         total_comment += stats[i].lines_comment;
         total_blank += stats[i].lines_blank;
     }
 
     fprintf(stream, "------------------------------------------------------\n");
     fprintf(stream, "Total      %7lu %8lu %9lu %8lu %8lu\n",
             total_files, total_code, total_comment, total_blank,
             total_code + total_comment + total_blank);
 
     return 0;
 }
 
 int get_supported_languages(lang_syntax_t *syntax, size_t max_langs) {
     size_t count = 0;
 
     if (!syntax || max_langs == 0) {
         return -1;
     }
 
     for (const lang_syntax_t *lang = languages; lang->name && count < max_langs; lang++, count++) {
         memcpy(&syntax[count], lang, sizeof(lang_syntax_t));
     }
 
     return (int)count;
 }
 
 const char *cloc_strerror(int error) {
     switch (error) {
         case CLOC_SUCCESS:
             return "Success";
         case CLOC_ERROR_IO:
             return "I/O error";
         case CLOC_ERROR_MEM:
             return "Memory allocation failed";
         case CLOC_ERROR_ARG:
             return "Invalid argument";
         case CLOC_ERROR_LIMIT:
             return "Internal limit reached";
         default:
             return "Unknown error";
     }
 }
 
 #ifndef CLOC_LIB_ONLY
 
 int main(int argc, char *argv[]) {
     int exit_code = 0;
 
     if (argc < 2) {
         fprintf(stderr, "Usage: %s <directory or file...>\n", argv[0]);
         return 1;
     }
 
     if (cloc_init() != CLOC_SUCCESS) {
         fprintf(stderr, "Failed to initialize\n");
         return 1;
     }
 
     for (int i = 1; i < argc; i++) {
         struct stat st;
         if (stat(argv[i], &st) == -1) {
             fprintf(stderr, "cloc: %s: %s\n", argv[i], strerror(errno));
             exit_code = 1;
             continue;
            }
    
            if (S_ISDIR(st.st_mode)) {
                if (process_directory(argv[i], update_stats_callback, NULL) != CLOC_SUCCESS) {
                    fprintf(stderr, "Error processing directory: %s\n", argv[i]);
                    exit_code = 1;
                }
            } else {
                file_stats_t stats;
                if (process_file(argv[i], &stats) == CLOC_SUCCESS) {
                    update_stats_callback(&stats, NULL);
                } else {
                    fprintf(stderr, "Error processing file: %s\n", argv[i]);
                    exit_code = 1;
                }
            }
        }
    
        print_stats(global_stats, global_stats_count, stdout);
        cloc_cleanup();
    
        return exit_code;
    }
    
    #endif /* CLOC_LIB_ONLY */