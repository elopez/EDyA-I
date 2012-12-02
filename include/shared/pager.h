#include <stdio.h>

#ifndef __H_PAGER__
#define __H_PAGER__

/**
 * Opens a stream similar to stdout, but paged using an available
 * system pager. In case no pager is available, it defaults to stdout.
 */
FILE *pager_init(void);

/**
 * Closes a pager stream, if necessary.
 * @param[in]   fp  The pager stream to close.
 */
int pager_close(FILE * fp);

#endif
