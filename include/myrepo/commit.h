#include <myrepo/hashtree.h>

#ifndef __H_COMMIT__
#define __H_COMMIT__

/*
 * Commits the current repository status for the files included on the
 * catalog.
 * @param[int]  message         A user-specified message to associate to the commit
 * @return 0 on success, 1 on error
 */
int myrepo_commit(const char *message);

/*
 * Loads the hash tree associated to a specific commit.
 * @param[in]   catalogpath     the catalog path
 * @param[in]   revision        the commit revision to use
 */
HashTreeNode *commit_loadtree(const char *catalogpath, unsigned int revision);

/*
 * Finds out the latest commit number on the repository and, optionally,
 * increments it by one
 * @param[in]   catalogpath     the catalog path
 * @param[in]   increment       increment the commit number by 1 (or not)
 */
unsigned int commit_latest(const char *catalogpath, int increment);

/*
 * Compares the current file against the one stored on a specific revision
 * @param[in]   catalogpath     the catalog path
 * @param[in]   revision        the commit revision to compare against
 * @param[in]   file            the file to compare
 * @return 0 if the file was not modified, any other value if it was.
 */
int commit_filestatus(const char *catalogpath, unsigned int revision,
                      const char *file);

/*
 * Generates a file as it was on a specific revision.
 * @param[in]   catalogpath     the catalog path
 * @param[in]   revision        the commit revision to use
 * @param[in]   file            the file name/path to generate
 * @param[out]  fcontents       the file contents as stored on that revision
 * @return the file line length
 */
unsigned int commit_file(const char *catalogpath, unsigned int revision,
                         const char *file, char ***fcontents);

/*
 * Diffs the current file against the one stored on a specific revision.
 * Depending on fp value, it will store the diff (commit mode, NULL fp) or
 * show it via fp
 * @param[in]   catalogpath     the catalog path
 * @param[in]   revision        the commit revision to diff against
 * @param[in]   file            the file to diff
 * @param[in]   fp              NULL for commit mode, or a FILE * to print to
 * @return 0 on success, any other value on error
 */
int commit_diff(char *catalogpath, unsigned int revision, const char *file,
                FILE * fp);

#endif
