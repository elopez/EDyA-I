#include <stdio.h>
#include <stdlib.h>

#include <shared/diff.h>
#include <shared/readfile.h>

int main(int argc, char *argv[])
{
    unsigned int alength, blength;
    char **alines, **blines;
    struct rule *rules;
    int status;

    if (argc != 3 && argc >= 1) {
        fprintf(stderr, "Usage: %s <original> <modified>\n", argv[0]);
        return 1;
    }

    FILE *filea = fopen(argv[1], "r");
    FILE *fileb = fopen(argv[2], "r");

    if (filea == NULL || fileb == NULL) {
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

    status = diff_lines(&rules, alines, alength, blines, blength);

    if (status == DIFF_ERROR) {
        fprintf(stderr, "Error computing differences.\n");
        return 4;
    }

    /* Don't print rules if the files are identical */
    if (status != DIFF_SAME) {
        diff_print(stdout, rules, alines, blines);
        diff_free_rules(rules);
    }

    /* Free and close everything */
    freereadfile(alines);
    freereadfile(blines);
    fclose(filea);
    fclose(fileb);

    return 0;
}
