#include <stdio.h>

#ifndef __H_ADDRM__
#define __H_ADDRM__

/**
 * Callback type for myrepo_recursive.
 * @param[in]   file        the filename to be used
 * @param[in]   extra       an extra pointer for any needed external information
 */
typedef void (*callback) (char *file, void *extra);

/**
 * Callback-typed function to add a file to the catalog.
 * @param[in]   name        the file name to add to the catalog
 * @param[in]   catalog     a FILE * to the catalog
 */
void myrepo_add(char *name, void *catalog);

/**
 * Callback-typed function to remove a file from the catalog.
 * @param[in]   name        the file name to remove from the catalog
 * @param[in]   catalog     a FILE * to the catalog
 */
void myrepo_remove(char *name, void *catalog);

/**
 * Callback-typed function to print the current status of a file
 * @param[in]   name        the file name to analyze
 * @param[in]   catalog     a FILE * to the catalog
 */
void myrepo_untracked(char *name, void *catalog);

/**
 * Run a callback function on all the files listed on filename and
 * its children if applicable.
 * @param[in]   filename    a NULL-terminated array of paths
 * @param[in]   function    the callback to execute
 * @param[in]   extra       an optional pointer handed to the callback
 * @return 0 on success, any other value on error
 */
int myrepo_recursive(char **filename, callback function, void *extra);

#endif
