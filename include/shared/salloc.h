#ifndef __H_SALLOC__
#define __H_SALLOC__

/*
 * Safe malloc implementation.
 * This will abort program execution if the allocation cannot be
 * fulfilled as requested.
 * @param[in]   size    size of the allocation
 * @return pointer to the allocation
 */
void *smalloc(size_t size);

/*
 * Safe calloc implementation.
 * This will abort program execution if the allocation cannot be
 * fulfilled as requested.
 * @param[in]   nmemb   number of members to allocate
 * @param[in]   size    size of each member
 * @return pointer to the allocation
 */
void *scalloc(size_t nmemb, size_t size);

/*
 * Safe realloc implementation.
 * This will abort program execution if the allocation cannot be
 * fulfilled as requested.
 * @param[in]   ptr     original pointer to be reallocated
 * @param[in]   size    new allocation size
 * @return pointer to the allocation
 */
void *srealloc(void *ptr, size_t size);

#endif
