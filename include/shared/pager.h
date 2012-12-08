#include <stdio.h>

#ifndef __H_PAGER__
#define __H_PAGER__

/**
 * Opens a stream similar to stdout, but paged using an available
 * system pager. In case no pager is available, it defaults to stdout.
 * @return a valid FILE * to output to
 */
FILE *pager_init(void);

/**
 * Closes a pager stream, if necessary.
 * @param[in]   fp  The pager stream to close.
 * @return 0 on success, any other value on error
 */
int pager_close(FILE * fp);

#endif
