#ifndef __H_HASH__
#define __H_HASH__

/**
 * Generates a SHA1 hash of the contents of a file
 * @param[in]   path        the file to hash using SHA1
 * @return the ASCII hex representation of the hash, or the following
 *         string on error: "deadda1adeadda1adeadda1adeadda1adeadda1a"
 */
char *hash_file(char *path);

#endif
