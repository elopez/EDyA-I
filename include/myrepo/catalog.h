#include <myrepo/hashtree.h>

#ifndef __H_CATALOG__
#define __H_CATALOG__

/*
 * Locates the root of a repository, where a catalog is found.
 * @return the path to the root of the repository, or NULL if you are
 *         not inside of one or an error occurs.
 */
char *catalog_locate(void);

/*
 * Opens the current repository catalog
 * @return the opened catalog, or NULL if you are not inside of
 *         of a repository.
 */
FILE *catalog_open(void);

/*
 * Check if a specific file is registered on our catalog
 * @param[in]   fp          the opened catalog
 * @param[in]   file        the file name/path to check
 * @return 0 if it is not registered, the line number on the catalog
 *           file if it is
 */
int catalog_exists(FILE * fp, const char *file);

/*
 * Appends a file name/path to our catalog if it is not registered
 * already.
 * @param[in]   fp          the opened catalog
 * @param[in]   file        the file name/path to add
 */
void catalog_add(FILE * fp, const char *file);

/*
 * Removes a name/path from our catalog if it is registered on it.
 * @param[in]   fp          the opened catalog
 * @param[in]   file        the file name/path to add
 */
void catalog_remove(FILE * fp, const char *file);

/*
 * Generates a hash tree from the current catalog
 * @param[in]   fp          the opened catalog
 * @param[out]  copytree    a copy of the generated tree
 * @param[in]   store       whether we should store the tree on a file
 * @return the root tree hash, or NULL on error.
 */
char *catalog_hash(FILE * fp, HashTreeNode ** copytree, int store);

#endif
