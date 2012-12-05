#ifndef __H_CLEANUP__
#define __H_CLEANUP__

/**
 * Register an element to be destroyed when finalizing execution successfully
 * @param[in]   element     The element to be destroyed
 * @param[in]   function    The function that will be called to destroy it
 */
void cleanup_register(void *element, void (*function) (void *element));

/**
 * Destroys all the elements registered using cleanup_register. Usually
 * you would use this as a parameter to atexit()
 */
void cleanup_execute(void);

#endif
