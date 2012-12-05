#include <stdio.h>

#ifndef __H_READFILE__
#define __H_READFILE__

#define READFILE_DEFAULT_LINES 50

/**
 * Reads a file to memory
 * @param[in]   file        FILE * as obtained with fopen
 * @param[out]  count       (optional) the number of read lines
 * @return the file contents, or NULL on error
 */
char **readfile(FILE * file, unsigned int *count);

/**
 * Frees a file from memory
 * @param[in]   contents    The file contents to free
 */
void freereadfile(char **contents);

#endif
