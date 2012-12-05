#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <shared/mkpath.h>

int mkpath(char *path, mode_t mode)
{
    unsigned int i = 1;         /* Ignore first / on full paths */

    assert(path != NULL);

    /* Navigate through the directories on the path and mkdir them */
    while (1) {
        while (path[i] != '\0' && path[i] != '/')
            i++;

        if (path[i] == '\0')
            return 0;

        path[i] = '\0';
        if (mkdir(path, mode) == -1 && errno != EEXIST) {
            /* Abort on error */
            path[i] = '/';
            return -1;
        }
        path[i] = '/';
        i++;
    }

    /* We should never reach this */
    return -1;
}
