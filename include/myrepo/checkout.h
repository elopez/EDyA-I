#ifndef __H_CHECKOUT__
#define __H_CHECKOUT__

/**
 * Restores the repo to a specific revision
 * @param[in]   revision        The revision to go back to
 * @return 0 on success, any other value if an error occured
 */
int myrepo_checkout(unsigned int revision);

#endif
