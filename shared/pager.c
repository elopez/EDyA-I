#include <stdio.h>

#include <shared/pager.h>

FILE *pager_init(void)
{
    FILE *fp;

    /* Try to execute a pager */
    fp = popen("sensible-pager || less || more || cat 2>/dev/null", "w");
    if (fp == NULL || fp == (FILE *) - 1)
        return stdout;

    return fp;
}

int pager_close(FILE * fp)
{
    if (fp == stdout)
        return 0;

    return pclose(fp);
}
