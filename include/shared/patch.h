#include <stdio.h>

#ifndef __H_PATCH__
#define __H_PATCH__

#define PATCH_OK    (1 << 0)
#define PATCH_ERROR (1 << 1)

int patch_file(char ***, unsigned int, char **, unsigned int);

#endif
