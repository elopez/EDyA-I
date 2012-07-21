#include <stdio.h>

#ifndef __H_ADDRM__
#define __H_ADDRM__

typedef void (*callback)(char* file, void* extra);

int myrepo_recursive(char **filename, callback function, void* extra);

/* Wrappers to catalog */
void myrepo_add(char*, void*);
void myrepo_remove(char*, void*);

#endif
