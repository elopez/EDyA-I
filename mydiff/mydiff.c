#include <stdio.h>
#include <stdlib.h>

#include <libs/diff.h>
#include <libs/readfile.h>

int main(int argc, char *argv[])
{
    unsigned int alength, blength;
    char **alines, **blines;
    struct rule *rules;

    if (argc != 3 && argc >= 1)
    {
        fprintf(stderr, "Usage: %s <original> <modified>\n", argv[0]);
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

    rules = diff_lines(alines, alength, blines, blength);

    if (rules == NULL) {
        fprintf(stderr, "Error computing differences / the files are identical.\n");
        return 4;
    }

    diff_print(stdout, rules, alines, blines);

    /* Free and close everything */
    freereadfile(alines);
    freereadfile(blines);
    fclose(filea);
    fclose(fileb);

    return 0;
}
