#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <shared/salloc.h>
#include <shared/readfile.h>

char **readfile(FILE * file, unsigned int *count)
{
    unsigned int i;
    unsigned int lineqty = READFILE_DEFAULT_LINES;
    char **lines = smalloc(READFILE_DEFAULT_LINES * sizeof(char *));
    size_t linesize = 0;

    /* files should always be valid */
    assert(file != NULL);

    for (i = 0; !feof(file); i++) {
        /* we ran out of space, grow our array */
        if (i == lineqty) {
            lineqty += READFILE_DEFAULT_LINES;
            lines = srealloc(lines, lineqty * sizeof(char *));
        }

        /* read a line and handle any errors appropriately */
        lines[i] = NULL;
        if (getline(&lines[i], &linesize, file) == -1) {
            free(lines[i]);
            lines[i--] = NULL;

            if (!feof(file)) {
                freereadfile(lines);
                return NULL;
            }
        }
    }

    /* if we're here it means we read all lines correctly */
    if (count != NULL)
        *count = i;

    lines[i] = NULL;

    return lines;
}

void freereadfile(char **lines)
{
    unsigned int i;

    /* pointers should always be valid */
    assert(lines != NULL);

    for (i = 0; lines[i] != NULL; i++)
        free(lines[i]);

    free(lines);
}
