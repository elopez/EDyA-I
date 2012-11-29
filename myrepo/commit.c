#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>

#include <shared/patch.h>
#include <shared/diff.h>
#include <shared/mkpath.h>
#include <shared/readfile.h>

#include <myrepo/catalog.h>
#include <myrepo/commit.h>

#include <myrepo/hashtree.h>

static int commit_diff(char *catalogpath, unsigned int rev, char *file);

int myrepo_commit(const char *message)
{
    FILE* catalog;
    FILE* commit;
    char * catalogpath = catalog_locate();
    char * cataloghash;
    char * revpath;
    const char ** differences;
    HashTreeNode * old;
    HashTreeNode * new;
    unsigned int revision;

    
    if (catalogpath == NULL)
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
    
    /* Hash the catalog and store it to disk */
    cataloghash = catalog_hash(catalog, &new, 1);
    
    fclose(catalog);
    
    /* Find out latest commit number and ++ it */
	revision = commit_latest(catalogpath, 1);
    
    /* Store commit message and match version and hash */
    revpath = (char*) malloc(strlen(catalogpath) + 100);
    sprintf(revpath, "%s/.index/revs/%d.rev", catalogpath, revision);
    commit = fopen(revpath, "w");
    if (commit == NULL)
    {
        fprintf(stderr, "Failed generating commit file\n");
        return 1;
    }
    
    if (revision == 1)
    {
        old = hashtree_new(); /* empty tree */
    } else {
		old = commit_loadtree(catalogpath, revision-1);
    }
    
    /* Compare the trees */
    differences = hashtree_compare(old, new);
    
    if (differences == NULL)
    {
        printf("No changes on this commit.\n");
    } else {
        for (int i=0; differences[i] != NULL; i++) {
            fprintf(commit, "involved=%s\n", differences[i]);
			commit_diff(catalogpath, revision, differences[i]);
		}
    }

    fprintf(commit, "hash=%s\ndate=%u\n%s\n", cataloghash,
        (unsigned int) time(NULL), message);
    fclose(commit);
    
    free(revpath);
    
    return 0;
}

HashTreeNode * commit_loadtree(const char *catalogpath, unsigned int revision)
{
	FILE* fd;
	HashTreeNode* tree;
    char tmphash[50];
	char* revpath = (char*) malloc(strlen(catalogpath) + 100);
	
    /* Revision 0 is an empty tree */
    if (revision == 0)
        return hashtree_new();
    
	/* Find out revision hash */
    sprintf(revpath, "%s/.index/revs/%d.rev", catalogpath, revision);
    fd = fopen(revpath, "r+");
    if (fd == NULL)
    {
        fprintf(stderr, "Failed to figure out old revision hash.\n");
        return NULL;
    }
    do {
        fgets(tmphash, 46, fd);
	} while (!feof(fd) && strncmp("hash=", tmphash, 5) != 0);
    fclose(fd);
        
    /* And load it, +5 for hash= */
    sprintf(revpath, "%s/.index/hashes/%s", catalogpath, tmphash+5);
    tree = hashtree_load(revpath);
    
    free(revpath);
    return tree;
}

unsigned int commit_latest(const char *catalogpath, int increment)
{
    FILE* fd;
    char *revpath;
    unsigned int revision;
    unsigned int len;

    len = strlen(catalogpath) + 100;
    revpath = (char*) malloc(len); /* TODO */
    snprintf(revpath, len, "%s/.index/revs/latest", catalogpath);
    fd = fopen(revpath, "r+");
    if (fd == NULL)
    {
        fprintf(stderr, "Failed to figure out revision number.\n");
        return 1;
    }
    fscanf(fd, "%u", &revision);
    
    if (increment) {
        revision++; /* TODO: overflow? */
        fd = freopen(revpath, "w", fd);
        fprintf(fd, "%u", revision);
    }
    
    fclose(fd);
    free(revpath);
    
    return revision;
}

int commit_filestatus(const char *catalogpath, unsigned int rev, const char *file)
{
	FILE* catalog;
	static HashTreeNode *new = NULL;
	static HashTreeNode *old = NULL;
	const char *oldhash;
	
	if (new == NULL) {
		catalog = catalog_open();
		if (catalog == NULL)
		{
			fprintf(stderr, "Failed to open repository catalog.\n");
			return 1;
		}
		catalog_hash(catalog, &new, 0);
		fclose(catalog);
	}
	
	if (old == NULL)
		old = commit_loadtree(catalogpath, rev);
	
	oldhash = hashtree_fetch(old, file);
	
	/* 0: up to date !0: modified */
	if (oldhash == NULL) /* freshly-added file, therefore modified */
		return 1;
	
	return strcmp(oldhash, hashtree_fetch(new, file));
}

static int commit_file_is_involved(const char *catalogpath, unsigned int rev, char *file)
{
	FILE* fd;
    char tmpread[50];
	char* revpath = (char*) malloc(strlen(catalogpath) + 100);
	
    /* No files are involved on revision 0 */
    if (rev == 0)
        return 0;
    
	/* Find out revision hash */
    sprintf(revpath, "%s/.index/revs/%d.rev", catalogpath, rev);
    fd = fopen(revpath, "r+");
    if (fd == NULL)
    {
        fprintf(stderr, "Failed to open revision data.\n");
        return 0;
    }
    
    do {
        fgets(tmpread, 46, fd);
	} while (!feof(fd) && strncmp("involved=", tmpread, 9) != 0);
    
    while (!feof(fd) && strncmp("involved=", tmpread, 9) == 0
        && strncmp(file, tmpread+9, strlen(file)) != 0) {
        fgets(tmpread, 46, fd);
	}

    fclose(fd);
    free(revpath);
    
    return !strncmp(file, tmpread+9, strlen(file));
}

unsigned int commit_file(const char *catalogpath, unsigned int rev, char *file, char ***fcontents)
{
	FILE *fp;
	unsigned int i;
    char *empty[] = {NULL};
	char **fileold = empty;
	char **filenew = empty;
	char **filepatch;
	unsigned int fileoldlen = 0;
	unsigned int filenewlen = 0;
	unsigned int filepatchlen;
	char *path;
	
	path = (char*) malloc(strlen(catalogpath)+strlen(file)+100); /* TODO? */
	
	for (i = 0; i <= rev; i++)
	{
		if(commit_file_is_involved(catalogpath, i, file))
		{
			sprintf(path, "%s/.index/patches/%u/%s.patch", catalogpath, i, file+2);
			fp = fopen(path, "r");
			filepatch = readfile(fp, &filepatchlen); /* TODO */
			patch_file(fileold, fileoldlen, filepatch, filepatchlen,
                &filenew, &filenewlen); /* TODO! */
            
            /* TODO: free! */
            fclose(fp);
            
            fileold = filenew;
            fileoldlen = filenewlen;
		}
	}
	
	*fcontents = filenew;
	return filenewlen;
}

static int commit_diff(char *catalogpath, unsigned int rev, char *file)
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
    
    path = (char*) malloc(strlen(catalogpath)+strlen(file)+100); /* TODO? */
    sprintf(path, "%s/.index/patches/%u/%s.patch", catalogpath, rev, file+2);
    mkpath(path, 0770); /* TODO */
	diff = fopen(path, "w"); /* TODO */
    
    flen = commit_file(catalogpath, rev-1, file, &fcontents);
    fcurrent = fopen(file, "r");
    if (fcurrent == NULL) /* TODO use stat? */
    {
        printf("Dropping deleted file from catalog: %s\n", file);
        fcurrent = catalog_open();
        catalog_remove(fcurrent, file);
        fclose(fcurrent);
        return 0;
    }
    fcurrcontents = readfile(fcurrent, &fcurrlen);
    fclose(fcurrent);
	
	status = diff_lines(&rules, fcontents, flen, fcurrcontents, fcurrlen);

    assert(status != DIFF_ERROR);
    assert(status != DIFF_SAME);

    diff_print(diff, rules, fcontents, fcurrcontents);
    
    fclose(diff);
    
    return 0;
}
