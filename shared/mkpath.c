#include <stdio.h>
#include <assert.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <shared/mkpath.h>

int mkpath(char *path, mode_t mode)
{
    unsigned int i;

    assert(path != NULL);

    for (;;) {
        while (path[i] != '\0' && path[i] != '/')
            i++;

        if (path[i] == '\0')
            return 0;

        path[i] = '\0';
        mkdir(path, mode); /* TODO: check for errors */
        path[i] = '/';
        i++;
    }
}
