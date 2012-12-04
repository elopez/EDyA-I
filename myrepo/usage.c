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
            " -  compare <path/to/repo1> <path/to/repo2>\n", name);
    return 1;
}
