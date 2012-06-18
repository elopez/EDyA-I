#include <libs/readfile.h>
#include <stdio.h>

void main(void)
{
    unsigned int i;
    char **lines;
    FILE* fp = fopen("/etc/hosts", "r");

    if (fp == NULL)
        return;

    lines = readfile(fp);

    if (lines == NULL)
        return;

    for(i = 0; lines[i] != NULL; i++)
        printf("%s", lines[i]);
}
