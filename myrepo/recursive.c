#define _POSIX_C_SOURCE 200112L

#include <assert.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <myrepo/catalog.h>
#include <myrepo/recursive.h>

int myrepo_recursive_step2(char *directory, callback function, void* extra)
{
    char *name[2];
    int dirlen = strlen(directory);
    DIR *dir;
    struct dirent *de;

    /* remove trailing / on directory names */
    while (dirlen > 1 && directory[dirlen-1] == '/')
    {
        directory[dirlen-1] = '\0';
        dirlen--;
    }

    if ((dir = opendir(directory)) == NULL)
    {
        fprintf(stderr, "%s: error opening directory\n", directory);
        return 1;
    }

    while((de = readdir(dir)) != NULL)
    {
        if (strcmp(".", de->d_name) == 0 ||
            strcmp("..", de->d_name) == 0 ||
            strcmp(".index", de->d_name) == 0)
            continue;

        name[0] = (char *) malloc((strlen(de->d_name) +
            dirlen + 4) * sizeof(char));
        sprintf(name[0], "%s/%s", directory, de->d_name);
        name[1] = NULL;

        /* add whatever we saw, if it is a directory it will
         * call us back */
        myrepo_recursive(name, function, extra);

        free(name[0]);
    }

    closedir(dir);
    return 0;
}

int myrepo_recursive(char **filename, callback function, void* extra)
{
    struct stat st;
    FILE* catalog;
    char* fname;

    /* filename is a null terminated array of strings, containing the
     * file names we should add to our index */

    /* the filename array must always contain at least 1 filename */
    assert(*filename != NULL);

    if (catalog_locate() == NULL)
    {
        fprintf(stderr, "You are not inside a repository, aborting.\n");
        return 1;
    }

    catalog = catalog_open();
    if (catalog == NULL)
    {
        fprintf(stderr, "Failed to open repository catalog.\n");
        return 1;
    }
    
    if (extra == NULL)
        extra = catalog;
    
    do {
        fname = (char *) malloc(strlen(*filename) + 2);
        if (fname == NULL)
            continue;

        /* always use ./file */
        if((*filename)[0] != '.' && (*filename)[1] != '/')
            sprintf(fname, "./%s", *filename);
        else
            sprintf(fname, "%s", *filename);

        if(stat(fname, &st) == 0)
        {
            if(S_ISDIR(st.st_mode))
                myrepo_recursive_step2(fname, function, extra);
            else if(S_ISREG(st.st_mode))
                function(fname, extra);
            else {
                fprintf(stderr, "%s: not a regular file or directory\n",
                    fname);
                return 1;
            }
        } else {
            fprintf(stderr, "%s: no such file or directory\n", fname);
            return 1;
        }

        filename++;
    } while (*filename != NULL);

    return 0;
}

void myrepo_add(char* name, void* extra)
{
    catalog_add((FILE*)extra, name);
}

void myrepo_remove(char* name, void* extra)
{
    catalog_add((FILE*)extra, name);
}