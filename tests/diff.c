#include <stdlib.h>

#include <shared/diff.h>
#include <shared/readfile.h>

int main(void)
{
    unsigned int alength, blength;
    char **alines, **blines;
    struct rule *rules;

    FILE* filea = fopen("testfile/diff1", "r");
    FILE* fileb = fopen("testfile/diff2", "r");

    if (filea == NULL || fileb == NULL)
        return 0;

    alines = readfile(filea, &alength);
    blines = readfile(fileb, &blength);

    if (alines == NULL || blines == NULL)
        return 0;

    diff_lines(&rules, alines, alength, blines, blength);

    diff_print(stdout, rules, alines, blines);

    return 0;
}
