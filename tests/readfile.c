#include <stdio.h>

#include <shared/readfile.h>

int main(void)
{
    unsigned int i;
    char **lines;
    FILE* fp = fopen("/etc/hosts", "r");

    if (fp == NULL)
        return 0;

    lines = readfile(fp, NULL);

    if (lines == NULL)
        return 0;

    for(i = 0; lines[i] != NULL; i++)
        printf("%s", lines[i]);

    return 0;
}
