#ifndef __H_ADDRM__
#define __H_ADDRM__

typedef void (*callback)(FILE* fp, char* file);

int myrepo_addrm(char **filename, callback function);

#endif
