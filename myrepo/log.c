#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <shared/pager.h>

#include <myrepo/catalog.h>
#include <myrepo/commit.h>
#include <myrepo/log.h>

int myrepo_log(void)
{
    FILE *fp;
    FILE *frev;
    char *catalogpath = catalog_locate();
    char *revpath;
    char tmpread[255];
    time_t time;
    unsigned int revision;

    if (catalogpath == NULL) {
        fprintf(stderr, "You are not inside a repository, aborting.\n");
        return 1;
    }

    /* Initialize the pager output and find out what's the latest commit
     * on the repository */
    fp = pager_init();
    revision = commit_latest(catalogpath, 0);

    revpath = (char *)malloc(strlen(catalogpath) + 100);
    if (revpath == NULL) {
        fprintf(stderr, "Error allocating memory for catalog path\n");
        return 1;
    }

    do {
        sprintf(revpath, "%s/.index/revs/%d.rev", catalogpath, revision);
        frev = fopen(revpath, "r");
        if (frev == NULL) {
            fprintf(stderr, "Failed fetching revision file\n");
            return 1;
        }

        /* Find out the corresponding root tree hash and print it */
        do {
            fgets(tmpread, 200, frev);
        } while (strncmp("hash=", tmpread, 5) != 0);

        fprintf(fp, "commit\t#%u %s", revision, tmpread + 5);

        /* Find out the corresponding date */
        do {
            fgets(tmpread, 200, frev);
        } while (strncmp("date=", tmpread, 5) != 0);

        /* Convert timestamp to text and print it */
        time = (time_t) atoi(tmpread + 5);
        strftime(tmpread, 200, "%c", localtime(&time));
        fprintf(fp, "Date:\t%s\n\n", tmpread);

        /* Finally print the commit message */
        while (!feof(frev) && fgets(tmpread, 200, frev))
            fprintf(fp, "\t%s", tmpread);

        fprintf(fp, "\n");

        fclose(frev);
    } while (--revision != 0);

    /* Close the pager and clean up */
    pager_close(fp);
    free(revpath);

    return 0;
}

int myrepo_logdiff(unsigned int revision)
{
    FILE *fp;
    FILE *catalog;
    char *catalogpath = catalog_locate();
    const char **differences;
    HashTreeNode *old;
    HashTreeNode *new;

    if (catalogpath == NULL) {
        fprintf(stderr, "You are not inside a repository, aborting.\n");
        return 1;
    }

    catalog = catalog_open();
    if (catalog == NULL) {
        fprintf(stderr, "Failed to open repository catalog.\n");
        return 1;
    }

    /* Initialize output stream */
    fp = pager_init();

    /* Hash the current catalog but do not store it to disk */
    catalog_hash(catalog, &new, 0);

    fclose(catalog);

    if (revision == 0) {
        old = hashtree_new();   /* empty tree */
        hashtree_compute(old);
    } else {
        old = commit_loadtree(catalogpath, revision);
        if (old == NULL)
            return 1;
    }

    /* Compare the trees */
    differences = hashtree_compare(old, new);

    if (differences == NULL) {
        printf("No changes.\n");
    } else {
        for (int i = 0; differences[i] != NULL; i++) {
            fprintf(fp, "File: %s\n", differences[i]);
            commit_diff(catalogpath, revision, differences[i], fp);
            fprintf(fp, "\n\n");
        }
        free(differences);
    }

    pager_close(fp);

    return 0;
}
