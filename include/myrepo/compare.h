#ifndef __H__COMPARE__
#define __H__COMPARE__

/**
 * Compares the last commited status between two catalogs
 * @param[in]   catalogpath1        The first catalog path
 * @param[in]   catalogpath2        The second catalog path
 * @return 0 on success, any other value on error
 */
int myrepo_compare(char *catalogpath1, char *catalogpath2);

#endif
