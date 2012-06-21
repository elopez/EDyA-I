#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <myrepo/catalog.h>

char *catalog_locate(void)
{
    struct stat st;
    static char *curdir = NULL;
    int i;

    /* cache the location */
    if (curdir != NULL)
        return curdir;

    /* Linux extension magic here */
    curdir = getcwd(NULL, 0);

    if (curdir == NULL)
        return NULL;

    /* check current dir */
    if (stat(".index", &st) == 0 && S_ISDIR(st.st_mode) &&
        stat(".index/contents", &st) == 0 && S_ISREG(st.st_mode))
        return curdir;

    /* visit every parent dir and check for a catalog */
    for(i=strlen(curdir)-1; i > 0; i--)
    {
        if (curdir[i] == '/')
        {
            curdir[i] = '\0';
            chdir(curdir);

            if (stat(".index", &st) == 0 && S_ISDIR(st.st_mode) &&
                stat(".index/contents", &st) == 0 && S_ISREG(st.st_mode))
            {
                /* found, return to starting point */
                curdir[i] = '/';
                chdir(curdir);
                curdir[i] = '\0';

                return curdir;
            }
        }
    }

    /* go back to where we started */
    chdir(curdir);
    free(curdir);

    return NULL;
}

FILE* catalog_open(void)
{
    char *catalogpath = catalog_locate();
    char *catalog;
    FILE* fp;

    if(catalogpath == NULL)
        return NULL;

    catalog = (char *) malloc((strlen(catalogpath) +
        strlen("/.index/contents")) * sizeof(char));
    sprintf(catalog, "%s/.index/contents", catalogpath);
    fp = fopen(catalog, "a+");
    free(catalog);

    return fp;
}

int catalog_exists(FILE* fp, char* file)
{
    int i;
    int linenr = 0;
    char *op;
    char *value;
    char line[1050];

    rewind(fp);

    while(!feof(fp))
    {
        linenr++;

        i = 0;
        fgets(line, 1049, fp);
        while(i < 1049 && line[i] != '=')
            i++;

        line[i] = '\0';
        op = line;
        value = line+i+1;
        value[strlen(value)-1] = '\0';

        if (strcmp(op, "file") == 0)
            if (strcmp(value, file) == 0)
                return linenr;
    }

    return 0;
}

void catalog_add(FILE* fp, char* file)
{
    if (!catalog_exists(fp, file))
        fprintf(fp, "file=%s\n", file);
}

void catalog_remove(FILE* fp, char* file)
{
    int i;
    int linenr;
    char *catalogpath = catalog_locate();
    char *catalog;
    char *oldcatalog;
    char line[1050];
    FILE* fpn;

    if(catalogpath == NULL)
        return;

    linenr = catalog_exists(fp, file);
    if (linenr == 0)
        return;

    catalog = (char *) malloc((strlen(catalogpath) +
        strlen("/.index/contents.tmp")) * sizeof(char));
    sprintf(catalog, "%s/.index/contents.tmp", catalogpath);
    fpn = fopen(catalog, "w+");

    if (fpn == NULL)
    {
        free(catalog);
        puts("BUG");
        return;
    }

    rewind(fp);

    /* copy lines up to the one to remove */
    for (i=1; i < linenr; i++)
    {
        fgets(line, 1049, fp);
        fprintf(fpn, "%s", line);
    }

    /* skip that one */
    fgets(line, 1049, fp);


    /* copy the rest */
    while (fgets(line, 1049, fp) != NULL)
        fprintf(fpn, "%s", line);

    fclose(fpn);

    /* remove .tmp */
    oldcatalog = malloc(strlen(catalog) * sizeof(char)); /* TODO */
    strncpy(oldcatalog, catalog, strlen(catalog)-4);

    /* replace the catalog atomically */
    rename(catalog, oldcatalog);
    freopen(oldcatalog, "a+", fp);
    free(oldcatalog);
    free(catalog);
}
