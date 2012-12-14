#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <shared/cleanup.h>

#include <myrepo/init.h>
#include <myrepo/usage.h>
#include <myrepo/catalog.h>
#include <myrepo/commit.h>
#include <myrepo/compare.h>
#include <myrepo/lock.h>
#include <myrepo/log.h>
#include <myrepo/recursive.h>

#define OPTION(name)    if(strcmp((name), argv[1]) == 0)

int main(int argc, char *argv[])
{
    /* Run our cleanup when we finish */
    atexit(cleanup_execute);

#ifdef WITH_LOCKS
    /* Make sure we are the only instance currently running */
    if (!myrepo_lock())
        return 0;

    /* Unlock at exit */
    atexit(myrepo_unlock);
#endif

    if (argc < 2 && argc >= 1)
        return myrepo_usage(argv[0]);

    OPTION("init") {
        return myrepo_init();
    }

    OPTION("add") {
        if (argc >=3)
            return myrepo_recursive(argv + 2, myrepo_add, NULL);
        else
            return myrepo_usage(argv[0]);
    }

    OPTION("rm") {
        if (argc >=3)
            return myrepo_recursive(argv + 2, myrepo_remove, NULL);
        else
            return myrepo_usage(argv[0]);
    }

    OPTION("status") {
        char *here[] = {".", NULL};
        return myrepo_recursive(here, myrepo_untracked, NULL);
    }

    OPTION("log") {
        if (argc >= 3)
            return myrepo_logdiff(atoi(argv[2]));
        else
            return myrepo_log();
    }

#ifndef SPEC_COMPLIANT
    OPTION("checkout") {
        if (argc >= 3)
            return myrepo_checkout(atoi(argv[2]));
        else
            return myrepo_usage(argv[0]);
    }
#endif

    OPTION("commit") {
        if (argc >= 4 && !strcmp(argv[2], "-m"))
            return myrepo_commit(argv[3]);
        else
            return myrepo_usage(argv[0]);
    }

    OPTION("compare") {
        if (argc >= 4)
            return myrepo_compare(argv[2], argv[3]);
        else
            return myrepo_usage(argv[0]);
    }

    return myrepo_usage(argv[0]);
}
