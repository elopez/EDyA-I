#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

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
    
    if (catalogpath == NULL)
    {
        fprintf(stderr, "You are not inside a repository, aborting.\n");
        return 1;
    }
    
    revision = commit_latest(catalogpath, 0);
    
    /* Try to output to a pager */
    fp = popen("sensible-pager || less || more || cat 2>/dev/null", "w");
    if (fp == NULL || fp == (FILE*)-1)
        fp = stdout;
    
    revpath = (char*) malloc(strlen(catalogpath) + 100); /* TODO */
    
    do {
        sprintf(revpath, "%s/.index/revs/%d.rev", catalogpath, revision);
        frev = fopen(revpath, "r");
        if (frev == NULL)
        {
            fprintf(stderr, "Failed fetching revision file\n");
            return 1;
        }
        
        do {
            fgets(tmpread, 200, frev);
		} while (strncmp("hash=", tmpread, 5) != 0);
        fprintf(fp, "commit\t#%u %s", revision, tmpread+5);
        
        do {
            fgets(tmpread, 200, frev);
		} while (strncmp("date=", tmpread, 5) != 0);
        time = (time_t) atoi(tmpread+5);
        strftime(tmpread, 200, "%c", localtime(&time));
        fprintf(fp, "Date:\t%s\n\n", tmpread);
        
        while(!feof(frev) && fgets(tmpread, 200, frev))
            fprintf(fp, "\t%s", tmpread);
        
        fprintf(fp, "\n");
        
        fclose(frev);
    } while (--revision != 0);
    
    if (fp != stdout)
        pclose(fp);
    
    free(revpath);
    
    return 0;
}
