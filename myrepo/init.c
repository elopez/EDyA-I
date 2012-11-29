#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include <myrepo/init.h>

int myrepo_init(void)
{
    struct stat st;
    FILE* fd;
    char *curdir;
    int i;

    if(stat(".index", &st) == 0)
    {
        if(S_ISDIR(st.st_mode))
            fprintf(stderr, "Repository already exists.\n");
        else
            fprintf(stderr, ".index is already being used, aborting.\n");

        return 1;
    }

    /* Linux extension magic here */
    curdir = getcwd(NULL, 0);

    if (curdir != NULL)
    {
        /* visit every parent dir and check for a repo; abort if found */
        for(i=strlen(curdir)-1; i > 0; i--)
        {
            if (curdir[i] == '/')
            {
                curdir[i] = '\0';
                chdir(curdir);
                if(stat(".index", &st) == 0 && S_ISDIR(st.st_mode))
                {
                    fprintf(stderr, "You are inside a repository. Aborting.\n");
                    free(curdir);
                    return 1;
                }
                curdir[i] = '/';
            }
        }

        chdir(curdir);
        free(curdir);
    }

    if(mkdir(".index", 0770) == -1 || mkdir(".index/revs", 0770) == -1
        || mkdir(".index/hashes", 0770) == -1)
    {
        fprintf(stderr, "Error creating index.\n");
        return 1;
    }

    fd = fopen(".index/contents", "w");

    if (fd == NULL)
    {
        fprintf(stderr, "Error creating catalog.\n");
        return 1;
    }

    fprintf(fd, "created=%u\n", (unsigned int) time(NULL));
    fclose(fd);

    fd = fopen(".index/revs/latest", "w");

    if (fd == NULL)
    {
        fprintf(stderr, "Error creating revision tracking file.\n");
        return 1;
    }

    fprintf(fd, "%d", 0);
    fclose(fd);

    fprintf(stdout, "Repository initialized correctly.\n");

    return 0;
}
