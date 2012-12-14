#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <shared/salloc.h>
#include <shared/mkpath.h>
#include <shared/readfile.h>

#include <myrepo/catalog.h>
#include <myrepo/commit.h>
#include <myrepo/checkout.h>

int myrepo_checkout(unsigned int revision)
{
    FILE *fp;
    char *catalogpath = catalog_locate();
    const char **differences;
    HashTreeNode *old;
    HashTreeNode *new;
    char **fcontents;
    int i, j;

    if (catalogpath == NULL) {
        fprintf(stderr, "You are not inside a repository, aborting.\n");
        return 1;
    }

    old = hashtree_new();       /* empty tree */
    hashtree_compute(old);

    if (revision == 0) {
        new = hashtree_new();   /* empty tree */
        hashtree_compute(new);
    } else {
        new = commit_loadtree(catalogpath, revision);
        if (new == NULL)
            return 1;
    }

    /* Compare the trees */
    differences = hashtree_compare(old, new);

    if (differences == NULL) {
        return 0;
    } else {
        for (i = 0; differences[i] != NULL; i++) {
            /* Create the path if missing and create/truncate the file */
            mkpath((char *)differences[i], 0777);
            fp = fopen(differences[i], "w");
            if (fp == NULL) {
                fprintf(stderr, "Error checking out %s\n", differences[i]);
                continue;
            }

            /* Fetch the file and write it to fp */
            commit_file(catalogpath, revision, differences[i], &fcontents);
            for (j = 0; fcontents[j] != NULL; j++)
                fprintf(fp, "%s", fcontents[j]);

            fclose(fp);
            freereadfile(fcontents);
        }
        free(differences);
    }

    return 0;
}
