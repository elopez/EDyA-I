#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <assert.h>

#include <shared/patch.h>
#include <shared/diff.h>
#include <shared/mkpath.h>
#include <shared/readfile.h>
#include <shared/salloc.h>

#include <myrepo/catalog.h>
#include <myrepo/commit.h>

#include <myrepo/hashtree.h>

int myrepo_commit(const char *message)
{
    FILE *catalog;
    FILE *commit;
    char *catalogpath = catalog_locate();
    char *cataloghash;
    char *revpath;
    const char **differences;
    HashTreeNode *old;
    HashTreeNode *new;
    unsigned int revision;

    if (catalogpath == NULL) {
        fprintf(stderr, "You are not inside a repository, aborting.\n");
        return 1;
    }

#ifdef SPEC_COMPLIANT
    revpath = getcwd(NULL, 0);  /* temporal use */
    if (revpath != NULL && strcmp(revpath, catalogpath)) {
        free(revpath);
        fprintf(stderr, "You are not on the repository root, aborting.\n");
        return 1;
    }
    free(revpath);
#endif

    catalog = catalog_open();
    if (catalog == NULL) {
        fprintf(stderr, "Failed to open repository catalog.\n");
        return 1;
    }

    /* Hash the catalog and store it to disk */
    cataloghash = catalog_hash(catalog, &new, 1);

    fclose(catalog);

    /* Find out latest commit number and ++ it */
    revision = commit_latest(catalogpath, 1);

    /* Store commit message and match version and hash */
    revpath = smalloc((strlen(catalogpath) + 100) * sizeof(char));
    sprintf(revpath, "%s/.index/revs/%d.rev", catalogpath, revision);
    commit = fopen(revpath, "w");
    if (commit == NULL) {
        fprintf(stderr, "Failed to generate commit file.\n");
        return 1;
    }

    /* Get the previous tree */
    if (revision == 1) {
        old = hashtree_new();   /* empty tree */
        hashtree_compute(old);
    } else {
        old = commit_loadtree(catalogpath, revision - 1);
    }

    /* Compare the trees */
    differences = hashtree_compare(old, new);

    if (differences == NULL) {
        printf("No changes on this commit.\n");
    } else {
        /* Indicate which files were involved and generate diffs for them */
        for (int i = 0; differences[i] != NULL; i++) {
            fprintf(commit, "involved=%s\n", differences[i]);
            commit_diff(catalogpath, revision - 1, differences[i], NULL);
        }
        free(differences);
    }

    /* Print the root hash of the tree, the date, and user message to finish */
    fprintf(commit, "hash=%s\ndate=%u\n%s\n", cataloghash,
            (unsigned int)time(NULL), message);
    fclose(commit);

    free(revpath);

    return 0;
}

HashTreeNode *commit_loadtree(const char *catalogpath, unsigned int revision)
{
    FILE *fd;
    HashTreeNode *tree;
    char tmphash[50];
    char *revpath;

    /* Revision 0 is an empty tree */
    if (revision == 0)
        return hashtree_new();  /* TODO? compute? */

    /* Find out revision hash */
    revpath = smalloc((strlen(catalogpath) + 100) * sizeof(char));
    sprintf(revpath, "%s/.index/revs/%d.rev", catalogpath, revision);
    fd = fopen(revpath, "r+");
    if (fd == NULL) {
        free(revpath);
        fprintf(stderr, "Failed to figure out old revision hash.\n");
        return NULL;
    }
    do {
        fgets(tmphash, 46, fd);
    } while (!feof(fd) && strncmp("hash=", tmphash, 5) != 0);
    fclose(fd);

    /* And load it, +5 for hash= */
    sprintf(revpath, "%s/.index/hashes/%s", catalogpath, tmphash + 5);
    tree = hashtree_load(revpath);

    free(revpath);
    return tree;
}

unsigned int commit_latest(const char *catalogpath, int increment)
{
    FILE *fd;
    char *revpath;
    unsigned int revision;

    /* Load the latest revision number from storage */
    revpath = smalloc((strlen(catalogpath) + 100) * sizeof(char));
    sprintf(revpath, "%s/.index/revs/latest", catalogpath);
    fd = fopen(revpath, "r+");
    if (fd == NULL) {
        fprintf(stderr, "Failed to find out latest revision number.\n");
        free(revpath);
        return 0;
    }
    fscanf(fd, "%u", &revision);

    /* Do we need to increment it? */
    if (increment) {
        revision++;             /* TODO: overflow? */
        fd = freopen(revpath, "w", fd);
        fprintf(fd, "%u", revision);
    }

    fclose(fd);
    free(revpath);

    return revision;
}

int commit_filestatus(const char *catalogpath, unsigned int revision,
                      const char *file)
{
    FILE *catalog;
    static HashTreeNode *new = NULL;
    static HashTreeNode *old = NULL;
    static unsigned int oldrev = 0;
    const char *oldhash;

    /* Hash the current tree once */
    if (new == NULL) {
        catalog = catalog_open();
        if (catalog == NULL) {
            fprintf(stderr, "Failed to open repository catalog.\n");
            return 1;
        }
        catalog_hash(catalog, &new, 0);
        fclose(catalog);
    }

    /* Load the old tree once, if revisions match */
    if (old == NULL && oldrev != revision) {
        old = commit_loadtree(catalogpath, revision);
        oldrev = revision;
    }

    /* Fetch the hash from the old tree */
    oldhash = hashtree_fetch(old, file);

    if (oldhash == NULL)        /* freshly-added file, therefore modified */
        return 1;

    /* 0: up to date !0: modified */
    return strcmp(oldhash, hashtree_fetch(new, file));
}

static int commit_file_is_involved(const char *catalogpath, unsigned int rev,
                                   const char *file)
{
    FILE *fd;
    char tmpread[50];
    char *revpath;

    /* No files are involved on revision 0 */
    if (rev == 0)
        return 0;

    revpath = smalloc((strlen(catalogpath) + 100) * sizeof(char));

    /* Find out revision hash */
    sprintf(revpath, "%s/.index/revs/%d.rev", catalogpath, rev);
    fd = fopen(revpath, "r+");
    if (fd == NULL) {
        fprintf(stderr, "Failed to open revision data.\n");
        free(revpath);
        return 0;
    }

    /* See if file is on any of the involved= lines */
    do {
        fgets(tmpread, 46, fd);
    } while (!feof(fd) && strncmp("involved=", tmpread, 9) != 0);

    while (!feof(fd) && strncmp("involved=", tmpread, 9) == 0
           && strncmp(file, tmpread + 9, strlen(file)) != 0) {
        fgets(tmpread, 46, fd);
    }

    fclose(fd);
    free(revpath);

    return !strncmp(file, tmpread + 9, strlen(file));
}

unsigned int commit_file(const char *catalogpath, unsigned int revision,
                         const char *file, char ***fcontents)
{
    FILE *fp;
    unsigned int i;
    char **fileold;
    char **filenew;
    char **filepatch;
    unsigned int fileoldlen;
    unsigned int filenewlen;
    unsigned int filepatchlen;
    char *path;

    fp = fopen("/dev/null", "r");   /* TODO */
    fileold = readfile(fp, &fileoldlen);
    filenew = fileold;
    filenewlen = fileoldlen;
    fclose(fp);

    path = smalloc((strlen(catalogpath) + strlen(file) + 100) * sizeof(char));

    /* Go from rev 1 to revision, and if file is involved in it, patch ours */
    for (i = 1; i <= revision; i++) {
        if (commit_file_is_involved(catalogpath, i, file)) {
            sprintf(path, "%s/.index/patches/%u/%s.patch", catalogpath, i,
                    file + 2);
            fp = fopen(path, "r");
            filepatch = readfile(fp, &filepatchlen);    /* TODO */
            patch_file(fileold, fileoldlen, filepatch, filepatchlen, &filenew, &filenewlen);    /* TODO! */

            fclose(fp);
            freereadfile(filepatch);
            freereadfile(fileold);

            fileold = filenew;
            fileoldlen = filenewlen;
        }
    }

    free(path);

    *fcontents = filenew;
    return filenewlen;
}

int commit_diff(char *catalogpath, unsigned int revision, const char *file,
                FILE * fp)
{
    FILE *fcurrent;
    FILE *diff;
    char *path;
    char **fcontents;
    char **fcurrcontents;
    int status;
    unsigned int flen;
    unsigned int fcurrlen;
    struct rule *rules;

    /* fp == NULL means we are in commit mode */
    if (fp == NULL) {
        path = smalloc((strlen(catalogpath) + strlen(file) + 100) *
                       sizeof(char));
        sprintf(path, "%s/.index/patches/%u/%s.patch", catalogpath,
                revision + 1, file + 2);
        mkpath(path, 0770);     /* TODO */
        diff = fopen(path, "w");    /* TODO */
        free(path);
    } else {                    /* fp != NULL means display mode */
        diff = fp;
    }

    flen = commit_file(catalogpath, revision, file, &fcontents);
    fcurrent = fopen(file, "r");
    if (fcurrent == NULL && fp == NULL) {   /* TODO use stat? */
        /* implicit myrepo rm: file is not there anymore, remove from catalog */
        printf("Dropping deleted file from catalog: %s\n", file);
        fcurrent = catalog_open();
        catalog_remove(fcurrent, file);
        freereadfile(fcontents);
        fclose(fcurrent);
        return 0;
    }

    /* TODO: what happens on display mode when file doesn't exist? */

    fcurrcontents = readfile(fcurrent, &fcurrlen);
    fclose(fcurrent);

    /* Diff file@revision and current file */
    status = diff_lines(&rules, fcontents, flen, fcurrcontents, fcurrlen);

    assert(status != DIFF_ERROR);
    assert(status != DIFF_SAME);

    diff_print(diff, rules, fcontents, fcurrcontents);

    diff_free_rules(rules);
    freereadfile(fcontents);
    freereadfile(fcurrcontents);

    if (fp == NULL)
        fclose(diff);

    return 0;
}
