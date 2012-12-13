#include <stdio.h>

#ifndef __H_PATCH__
#define __H_PATCH__

/* Return codes for patch_file */
#define PATCH_OK    (1 << 0)
#define PATCH_ERROR (1 << 1)

/**
 * Patches a file with a text patch
 * @param[in]   fileold         The origin file
 * @param[in]   filelen         The origin file's length
 * @param[in]   patch           The patch file
 * @param[in]   patchlen        The patch file's length
 * @param[out]  newfilep        A pointer to store the new file in
 * @param[out]  newlen          Optional, a pointer to store newfilep's length
 * @return PATCH_OK on success
 *         PATCH_ERROR if an error occured
 */
int patch_file(char **fileold, unsigned int filelen,
               char **patch, unsigned int patchlen,
               char ***newfilep, unsigned int *newlen);
#endif
