#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int myrepo_init(void)
{
    struct stat st;

    if(stat(".index", &st) == 0)
    {
        if(S_ISDIR(st.st_mode))
            fprintf(stderr, "Repository already exists.\n");
        else
            fprintf(stderr, ".index is already being used, aborting.\n");

        return 1;
    }

    if(mkdir(".index", 0770) == -1) {
        fprintf(stderr, "Error creating index.\n");
        return 1;
    }

    fprintf(stdout, "Repository initialized correctly.\n");

    return 0;
}
