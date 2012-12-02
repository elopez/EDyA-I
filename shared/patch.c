#include <stdlib.h>
#include <string.h>

#include <shared/patch.h>

/* Hunk types */
#define HUNK_NONE   (1 << 0)
#define HUNK_CHANGE (1 << 1)
#define HUNK_APPEND (1 << 2)
#define HUNK_REMOVE (1 << 3)

int patch_file(char ** fileold, unsigned int filelen,
                char ** patch, unsigned int patchlen,
                char *** newfilep, unsigned int *newlen)
{
    unsigned int i, j;
    unsigned int hunktype = HUNK_NONE;
    unsigned int oldstart, oldend, newstart, newend, count = 0;

    char ** file = (char**) calloc(filelen+1, sizeof(char *));
    if(filelen > 0)
        memcpy(file, fileold, sizeof(char *) * filelen);
    char ** newfile = (char **) calloc(filelen + patchlen, sizeof(char *));
    *newfilep = newfile;

    /* Parse patch file */
    for (i=0; i < patchlen; i++)
    {
        /* Hunk start */
        if (patch[i][0] >= '0' && patch[i][0] <= '9')
        {
            oldstart = atoi(patch[i]);
            j = 1;
            while (patch[i][j] != ',' && patch[i][j] != '\0' &&
                    patch[i][j] != 'a' && patch[i][j] != 'c' &&
                    patch[i][j] != 'd') j++;

            /* Malformed patch */
            if (patch[i][j] == '\0')
                return PATCH_ERROR;

            /* If we're on a comma, it's "1,2c.." style
             * Otherwise it's 1c.." style and start == end */
            if (patch[i][j] == ',')
                oldend = atoi(patch[i] + j + 1);
            else
                oldend = oldstart;

            while (patch[i][j] != '\0' && patch[i][j] != 'a' &&
                    patch[i][j] != 'c' && patch[i][j] != 'd') j++;

            if (patch[i][j] == 'a')
                hunktype = HUNK_APPEND;
            else if (patch[i][j] == 'd')
                hunktype = HUNK_REMOVE;
            else if (patch[i][j] == 'c')
                hunktype = HUNK_CHANGE;
            else
                return PATCH_ERROR;

            newstart = atoi(patch[i] + j + 1);

            while (patch[i][j] != ',' && patch[i][j] != '\0') j++;

            /* No comma, single digit */
            if (patch[i][j] == '\0')
                newend = newstart;
            else
                newend = atoi(patch[i] + j + 1);

            /* Well-formedness checks */
            if (newend < newstart || oldend < oldstart || oldend > filelen)
                return PATCH_ERROR;

            continue;
        /* Fast forward over the patch */
        } else if (hunktype == HUNK_NONE) {
            continue;
        /* Old lines */
        } else if (patch[i][0] == '<') {
            /* This is a *really* simple patch implementation, so
             * we don't actually check we're doing the right thing.
             * A proper patching program would compare the lines and
             * make sure it's deleting the correct ones. */
            if (hunktype == HUNK_REMOVE || hunktype == HUNK_CHANGE)
                for (j = oldstart; j <= oldend; j++)
                    file[j-1] = (char *) 0xDEADBEEF; /* TODO: free line? */

            /* Don't run this loop again */
            oldstart = oldend + 1;
            continue;
        /* New lines */
        } else if (patch[i][0] == '>') {
            if (newstart <= newend) {
                newfile[newstart-1] = malloc(strlen(patch[i])); /* TODO */
                sprintf(newfile[newstart-1], "%s", patch[i] + 2);
                newstart++;
            }
        /* Switch of hunk */
        } else if (patch[i][0] == '-') {
            continue;
        /* Empty tailing line */
        } else if (patch[i][0] == '\r' || patch[i][0] == '\n') {
            continue;
        /* End of patch */
        } else if (patch[i][0] == '\0') {
            break;
        } else {
            /* Malformed patch */
            return PATCH_ERROR;
        }
    }

    i = j = 0;
    /* Copy the not changed lines */
    while (file[i] != NULL && filelen--)
    {
        /* if it's a deleted line, skip it */
        if (file[i] == (char *) 0xDEADBEEF) {
            i++;
            continue;
        }

        /* find the next empty slot */
        while(newfile[j] != NULL)
            j++;

        /* Move the line */
        newfile[j] = strdup(file[i]);
        i++;
    }

    if(newlen != NULL) {
        while(newfile[count] != NULL)
            count++;
        *newlen = count;
    }

    /* Clean temporary file representation */
    free(file);

    return PATCH_OK;
}
