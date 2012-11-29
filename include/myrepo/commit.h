#include "hashtree.h"

#ifndef __H_COMMIT__
#define __H_COMMIT__

int myrepo_commit(const char *message);

HashTreeNode * commit_loadtree(const char *catalogpath, unsigned int revision);
unsigned int commit_latest(const char *catalogpath, int increment);
int commit_filestatus(const char *catalogpath, unsigned int rev, const char *file);
unsigned int commit_file(const char *catalogpath, unsigned int rev, char *file, char ***fcontents);

#endif
