#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <shared/cleanup.h>
#include <shared/salloc.h>

#include <myrepo/hashtree.h>
#include <myrepo/catalog.h>

char *catalog_locate(void)
{
    struct stat st;
    static char *curdir = NULL;
    char *origindir;
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
        stat(".index/contents", &st) == 0 && S_ISREG(st.st_mode)) {
        cleanup_register(curdir, free);
        return curdir;
    }

    origindir = strdup(curdir);

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
                chdir(origindir);
                free(origindir);

                cleanup_register(curdir, free);
                return curdir;
            }
        }
    }

    /* go back to where we started */
    chdir(origindir);
    free(origindir);
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

    catalog = (char *) smalloc((strlen(catalogpath) +
        strlen("/.index/contents") + 1) * sizeof(char));
    sprintf(catalog, "%s/.index/contents", catalogpath);
    fp = fopen(catalog, "a+");
    free(catalog);

    return fp;
}

int catalog_exists(FILE* fp, const char* file)
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
        while(i < 1049 && line[i] != '=' && line[i] != '\0')
            i++;

        /* malformed line */
        if (line[i] == '\0')
            continue;

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

void catalog_add(FILE* fp, const char* file)
{
    if (!catalog_exists(fp, file))
        fprintf(fp, "file=%s\n", file);
}

void catalog_remove(FILE* fp, const char* file)
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

    catalog = (char *) smalloc((strlen(catalogpath) +
        strlen("/.index/contents.tmp")) * sizeof(char));
    sprintf(catalog, "%s/.index/contents.tmp", catalogpath);
    fpn = fopen(catalog, "w+");

    if (fpn == NULL)
    {
        free(catalog);
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
    oldcatalog = strdup(catalog);
    oldcatalog[strlen(oldcatalog)-4] = '\0';

    /* replace the catalog atomically */
    rename(catalog, oldcatalog);
    freopen(oldcatalog, "a+", fp);
    free(oldcatalog);
    free(catalog);
}

char* catalog_hash(FILE* fp, HashTreeNode **copytree, int store)
{
    unsigned int i;
    char line[1050];
    char *op;
    char *value;
    char *hash;
    char *catalogpath = catalog_locate();
    char *dump;
    HashTreeNode* tree = hashtree_new();
    FILE* fpdump;

    if (tree == NULL)
        return NULL;

    chdir(catalogpath);

    rewind(fp);

    while(!feof(fp))
    {
        i = 0;
        fgets(line, 1049, fp);
        while(i < 1049 && line[i] != '=' && line[i] != '\0')
            i++;

        /* malformed line */
        if (line[i] == '\0')
            continue;

        line[i] = '\0';
        op = line;
        value = line+i+1;
        value[strlen(value)-1] = '\0';

        if (strcmp(op, "file") == 0)
            hashtree_insert(tree, value, NULL);
    }

    hash = hashtree_compute(tree);

    if (copytree != NULL)
        *copytree = tree;
    /* else
        TODO free tree? */

    if (!store)
        return hash;

    dump = malloc((strlen(catalogpath) + 60) * sizeof(char));
    if (dump == NULL)
        return NULL;

    sprintf(dump, "%s/.index/hashes/%s", catalogpath, hash);
    fpdump = fopen(dump, "w+");

    if (fpdump == NULL)
    {
        free(dump);
        free(hash);
        return NULL;
    }

    hashtree_print(tree, fpdump);

    fclose(fpdump);
    free(dump);

    return hash;
}
