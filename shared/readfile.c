#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libs/readfile.h>

char **readfile(FILE* file, unsigned int* count)
{
    unsigned int i;
    unsigned int lineqty = READFILE_DEFAULT_LINES;
    char **lines = (char **)malloc(READFILE_DEFAULT_LINES * sizeof(char *));
    size_t linesize = 0;
    void *newblock;

    /* files should always be valid */
    assert(file != NULL);

    for (i=0; !feof(file); i++)
    {
        /* we ran out of space, grow our array */
        if (i == lineqty)
        {
            lineqty += READFILE_DEFAULT_LINES;
            newblock = realloc(lines, lineqty * sizeof(char *));

            if (newblock != NULL)
            {
                /* all went well! */
                lines = (char **)newblock;
            } else {
                /* clean up, we cannot continue */
                lines[i-1] = NULL;
                freereadfile(lines);
                return NULL;
            }
        }

        /* read a line and handle any errors appropriately */
        lines[i] = NULL;
        if (getline(&lines[i], &linesize, file) == -1 && !feof(file))
        {
            lines[i] = NULL;
            freereadfile(lines);
            return NULL;
        }
    }

    /* if we're here it means we read all lines correctly */
    if (count != NULL)
        *count = i;

    return lines;
}

void freereadfile(char** lines)
{
    int i;

    /* pointers should always be valid */
    assert(lines != NULL);

    for (i=0; lines[i] != NULL; i++)
        free(lines[i]);

    free(lines);
}
