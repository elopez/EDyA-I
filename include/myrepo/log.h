#ifndef __H_LOG__
#define __H_LOG__

/**
 * Prints the history of the repository, from most recent commit
 * to the oldest one.
 * @return 0 on success, other values on error.
 */
int myrepo_log(void);

/**
 * Computes the diff between a certain revision and the current
 * repository and prints it.
 * @param[in]   revision    The revision to use on the comparison.
 * @return 0 on success, other values on error.
 */
int myrepo_logdiff(unsigned int revision);

#endif
