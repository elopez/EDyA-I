#include <assert.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <shared/salloc.h>

#include <myrepo/catalog.h>
#include <myrepo/commit.h>
#include <myrepo/recursive.h>

static int myrepo_recursive_step2(char *directory, callback function,
                                  void *extra)
{
    char *name[2];
    int dirlen = strlen(directory);
    DIR *dir;
    struct dirent *de;

    /* remove trailing / on directory names */
    while (dirlen > 1 && directory[dirlen - 1] == '/') {
        directory[dirlen - 1] = '\0';
        dirlen--;
    }

    if ((dir = opendir(directory)) == NULL) {
        fprintf(stderr, "%s: error opening directory\n", directory);
        return 1;
    }

    while ((de = readdir(dir)) != NULL) {
        if (strcmp(".", de->d_name) == 0 ||
            strcmp("..", de->d_name) == 0 || strcmp(".index", de->d_name) == 0)
            continue;

        name[0] = (char *)smalloc((strlen(de->d_name) +
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

int myrepo_recursive(char **filename, callback function, void *extra)
{
    struct stat st;
    char *fname;
    char *cwd;
    char *relative;
    FILE *catalog;
    static char *catalogpath;

    /* filename is a null terminated array of strings, containing the
     * file names we should add to our index */

    /* the filename array must always contain at least 1 filename */
    assert(*filename != NULL);

    if (catalogpath == NULL) {
        catalogpath = catalog_locate();
        if (catalogpath == NULL) {
            fprintf(stderr, "You are not inside a repository, aborting.\n");
            return 1;
        }
    }

    catalog = catalog_open();
    if (catalog == NULL) {
        fprintf(stderr, "Failed to open repository catalog.\n");
        return 1;
    }

    if (extra == NULL)
        extra = catalog;

    /* Where are we? */
    cwd = getcwd(NULL, 0);
    relative = cwd + strlen(catalogpath);
    chdir(catalogpath);

    do {
        fname = smalloc((strlen(*filename) + strlen(relative) + 2) *
                        sizeof(char));
        if (fname == NULL)
            continue;

        /* always use ./file */
        if ((*filename)[0] == '.' && (*filename)[1] == '\0')
            sprintf(fname, ".%s", relative);
        else if ((*filename)[0] == '.' && (*filename)[1] == '/')
            sprintf(fname, ".%s/%s", relative, *filename + 2);
        else
            sprintf(fname, ".%s/%s", relative, *filename);

        if (stat(fname, &st) == 0) {
            if (S_ISDIR(st.st_mode))
                myrepo_recursive_step2(fname, function, extra);
            else if (S_ISREG(st.st_mode))
                function(fname, extra);
            else {
                fprintf(stderr, "%s: not a regular file or directory\n", fname);
                free(cwd);
                free(fname);
                return 1;
            }
        } else {
            fprintf(stderr, "%s: no such file or directory\n", fname);
            free(cwd);
            free(fname);
            return 1;
        }

        filename++;
    } while (*filename != NULL);

    chdir(cwd);
    free(cwd);
    free(fname);
    fclose(catalog);

    return 0;
}

void myrepo_add(char *name, void *extra)
{
    catalog_add((FILE *) extra, name);
}

void myrepo_remove(char *name, void *extra)
{
    catalog_remove((FILE *) extra, name);
}

void myrepo_untracked(char *name, void *extra)
{
    static char *catalogpath = NULL;
    static unsigned int latest;

    if (catalogpath == NULL) {
        catalogpath = catalog_locate();
        latest = commit_latest(catalogpath, 0);
    }

    if (!catalog_exists((FILE *) extra, name))
        printf("Untracked:\t%s\n", name);
    else if (commit_filestatus(catalogpath, latest, name))
        printf("Modified:\t%s\n", name);
    else
        printf("Up to date:\t%s\n", name);
}
