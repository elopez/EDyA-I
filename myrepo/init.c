#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

int myrepo_init(void)
{
    struct stat st;
    FILE* fd;

    if(stat(".index", &st) == 0)
    {
        if(S_ISDIR(st.st_mode))
            fprintf(stderr, "Repository already exists.\n");
        else
            fprintf(stderr, ".index is already being used, aborting.\n");

        return 1;
    }

    if(mkdir(".index", 0770) == -1 || mkdir(".index/revs", 0770) == -1)
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

    fprintf(stdout, "Repository initialized correctly.\n");

    return 0;
}
