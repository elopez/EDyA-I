#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <shared/salloc.h>

void *smalloc(size_t size)
{
    void *ptr = malloc(size);

    if (ptr != NULL)
        return ptr;

    fprintf(stderr,
            "A memory allocation failed. The program will now abort execution.\n");

    exit(4);
}

void *scalloc(size_t nmemb, size_t size)
{
    void *ptr = calloc(nmemb, size);

    if (ptr != NULL)
        return ptr;

    fprintf(stderr,
            "A memory allocation failed. The program will now abort execution.\n");

    exit(4);
}

void *srealloc(void *ptr, size_t size)
{
    void *newptr = realloc(ptr, size);

    if (newptr != NULL)
        return newptr;

    fprintf(stderr,
            "A memory relocation failed. The program will now abort execution.\n");

    exit(4);
}

char *sstrdup(const char *s)
{
    char *news = strdup(s);

    if (news != NULL)
        return news;

    fprintf(stderr,
            "A string duplication failed. The program will now abort execution.\n");

    exit(4);
}
