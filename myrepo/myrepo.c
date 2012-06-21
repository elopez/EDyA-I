#include <stdio.h>
#include <string.h>

#include <myrepo/addrm.h>
#include <myrepo/init.h>
#include <myrepo/usage.h>
#include <myrepo/catalog.h>

#define OPTION(name)    if(strcmp((name), argv[1]) == 0)

int main(int argc, char *argv[])
{
    if (argc < 2 && argc >= 1)
        return myrepo_usage(argv[0]);

    OPTION("init") {
        return myrepo_init();
    }

    OPTION("add") {
        if (argc >=3)
            return myrepo_addrm(argv + 2, catalog_add);
        else
            return myrepo_usage(argv[0]);
    }

    OPTION("rm") {
        if (argc >=3)
            return myrepo_addrm(argv + 2, catalog_remove);
        else
            return myrepo_usage(argv[0]);
    }

    OPTION("status") {
        puts("status");
        return 0;
    }

    OPTION("log") {
        puts("log");
        return 0;
    }

    OPTION("commit") {
        puts("commit");
        return 0;
    }

    OPTION("compare") {
        puts("compare");
        return 0;
    }

    return myrepo_usage(argv[0]);
}
