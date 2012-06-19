#include <stdio.h>
#include <string.h>

#define OPTION(letter, name, tocheck) \
    case (letter): if (strcmp((name), (tocheck)) == 0)
#define OPTION2(letter, name, tocheck) \
    if(strcmp((name), (tocheck)) == 0)

int main(int argc, char *argv[])
{
    if (argc < 2 && argc >= 1)
        return myrepo_usage(argv[0]);

    switch (argv[1][0])
    {
        OPTION('i', "init", argv[1]) {
            return myrepo_init();
        }

        OPTION('a', "add", argv[1]) {
            puts("add");
            break;
        }

        OPTION('r', "rm", argv[1]) {
            puts("rm");
            break;
        }

        OPTION('s', "status", argv[1]) {
            puts("status");
            break;
        }

        OPTION('l', "log", argv[1]) {
            puts("log");
            break;
        }

        OPTION('c', "commit", argv[1]) {
            puts("commit");
        }

        OPTION2('c', "compare", argv[1]) {
            puts("compare");
            break;
        }

        default:
            return myrepo_usage(argv[0]);
    }

}
