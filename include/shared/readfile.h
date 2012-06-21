#include <stdio.h>

#ifndef __H_READFILE__
#define __H_READFILE__

#define READFILE_DEFAULT_LINES 50

/*  readfile: reads a file to memory
 *
 *  arguments: FILE* as obtained with fopen
 *             unsigned int * (optional) to be updated with the
 *                 number of read lines
 *
 *  returns: NULL on error
 *           char ** with the lines on success
 *
 *  notes: you can free the char** with freereadfile
 */
char ** readfile(FILE*, unsigned int *);

/*  freereadfile: frees a file on memory
 *
 *  arguments: char ** as obtained with readfile
 *
 */
void freereadfile(char **);

#endif
