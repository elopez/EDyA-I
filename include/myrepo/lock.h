#ifndef __H_LOCK__
#define __H_LOCK__

/**
 * Prevents other instances of myrepo from running and creates a flag file
 * for repository integrity check on future runs.
 * @return 0 if an error occured, any other value on success.
 */
int myrepo_lock(void);

/**
 * Unlinks the integrity check flag file. Intended to be run on normal exit
 * via atexit(). The myrepo lock will be automatically unlocked when the
 * process exits.
 */
void myrepo_unlock(void);

#endif
