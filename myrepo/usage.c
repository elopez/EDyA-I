#include <stdio.h>

#include <myrepo/usage.h>

int myrepo_usage(char *name)
{
    fprintf(stderr, "Usage: %s \n"
            " -  init\n"
            " -  add <file/directory> (<file/directory> ..)\n"
            " -  rm <file/directory> (<file/directory> ..)\n"
            " -  status\n"
            " -  commit -m \"commit message\"\n"
            " -  log (<revision>)\n"
#ifndef SPEC_COMPLIANT
            " -  checkout <revision>\n"
#endif
            " -  compare <path/to/repo1/.index> <path/to/repo2/.index>\n",
            name);

    return 1;
}
