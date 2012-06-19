#include <stdio.h>

int myrepo_usage(char *name)
{
    fprintf(stderr, "Usage: %s \n"
                    " -  init\n"
                    " -  add <file/directory> (<file/directory> ..)\n"
                    " -  rm <file/directory> (<file/directory> ..)\n"
                    " -  status\n"
                    " -  commit\n"
                    " -  log (<revision>)\n"
                    " -  compare <path/to/repo1> <path/to/repo2>\n", name);
    return 1;
}
