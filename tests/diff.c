#include <stdlib.h>

#include <libs/diff.h>
#include <libs/readfile.h>

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

    rules = diff_lines(alines, alength, blines, blength);

    diff_print(stdout, rules, alines, blines);

    return 0;
}
