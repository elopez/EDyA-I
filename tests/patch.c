#include <stdlib.h>
#include <assert.h>

#include <shared/patch.h>
#include <shared/readfile.h>

int main(void)
{
    unsigned int alength, blength, newlength;
    char **alines, **blines, **newfile;
    unsigned int i = 0;

    FILE* filea = fopen("testfile/diff1", "r");
    FILE* fileb = fopen("patchfile", "r");

    if (filea == NULL || fileb == NULL)
        return 0;

    alines = readfile(filea, &alength);
    blines = readfile(fileb, &blength);

    if (alines == NULL || blines == NULL)
        return 0;

    patch_file(alines, alength, blines, blength, &newfile, &newlength);

    while (newfile[i] != NULL)
        printf("%s", newfile[i++]);

    assert(i == newlength);

    return 0;
}
