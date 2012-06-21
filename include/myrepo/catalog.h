#ifndef __H_CATALOG__
#define __H_CATALOG__

char *catalog_locate(void);

FILE* catalog_open(void);
int catalog_exists(FILE* fp, char* file);
void catalog_add(FILE* fp, char* file);
void catalog_remove(FILE* fp, char* file);

#endif
