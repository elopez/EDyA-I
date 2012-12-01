#include "hashtree.h"

#ifndef __H_CATALOG__
#define __H_CATALOG__

char *catalog_locate(void);

FILE* catalog_open(void);
int catalog_exists(FILE* fp, const char* file);
void catalog_add(FILE* fp, const char* file);
void catalog_remove(FILE* fp, const char* file);
char* catalog_hash(FILE* fp, HashTreeNode** copytree, int store);

#endif
