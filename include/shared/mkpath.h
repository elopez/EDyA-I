#include <sys/types.h>

#ifndef __H_MKPATH__
#define __H_MKPATH__

/**
 * Generates a chain of directories. Equivalent to "mkdir -p". Will not
 * fail if the directories exist.
 * @param[in]   path        The chain of directories to create
 * @param[in]   mode        The permissions to use. Subject to umask
 * @return 0 on success, -1 on error. errno will contain the mkdir(...) error
 */
int mkpath(char *path, mode_t mode);

#endif
