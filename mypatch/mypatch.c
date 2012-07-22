#include <stdio.h>
#include <stdlib.h>

#include <shared/patch.h>
#include <shared/readfile.h>

int main(int argc, char *argv[])
{
    unsigned int alength, blength;
    char **alines, **blines;
    int status;

    if (argc != 3 && argc >= 1)
    {
        fprintf(stderr, "Usage: %s <original> <patch>\n", argv[0]);
        return 1;
    }

    FILE* filea = fopen(argv[1], "r");
    FILE* fileb = fopen(argv[2], "r");

    if (filea == NULL || fileb == NULL)
    {
        fprintf(stderr, "Error opening files.\n");

        /* Close any possibly open file */
        if (filea != NULL)
            fclose(filea);
        if (fileb != NULL)
            fclose(fileb);

        return 2;
    }

    /* Read the files to memory */
    alines = readfile(filea, &alength);
    blines = readfile(fileb, &blength);

    if (alines == NULL || blines == NULL) {
        fprintf(stderr, "Error reading files.\n");

        /* Free any possibly used resources */
        if (alines != NULL)
            freereadfile(alines);
        if (blines != NULL)
            freereadfile(blines);

        fclose(filea);
        fclose(fileb);

        return 3;
    }

    status = patch_file(&alines, alength, blines, blength);

    if (status == PATCH_ERROR) {
        fprintf(stderr, "Error patching.\n");
        return 4;
    }

    /* Write patched contents */
    unsigned int i = 0;
    filea = freopen(argv[1], "w+", filea);
    while(alines[i] != NULL)
        fprintf(filea, "%s", alines[i++]);


    /* Free and close everything */
    //freereadfile(alines);
    //freereadfile(blines);
    fclose(filea);
    fclose(fileb);

    return 0;
}
