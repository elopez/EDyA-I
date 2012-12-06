#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <libgen.h>

#include <myrepo/compare.h>
#include <myrepo/commit.h>
#include <myrepo/hashtree.h>

int myrepo_compare(char *catalogpath1, char *catalogpath2)
{
    unsigned int latest1;
    unsigned int latest2;
    HashTreeNode *tree1;
    HashTreeNode *tree2;

    assert(catalogpath1 != NULL);
    assert(catalogpath2 != NULL);

    catalogpath1 = strdup(catalogpath1);
    catalogpath2 = strdup(catalogpath2);

    /* Remove any possible end slash */
    catalogpath1[strlen(catalogpath1) - 2] = '\0';
    catalogpath2[strlen(catalogpath2) - 2] = '\0';

    /* Remove last component (.index) */
    catalogpath1 = dirname(catalogpath1);
    catalogpath2 = dirname(catalogpath2);

    latest1 = commit_latest(catalogpath1, 0);

    /* The directory is likely not a valid repo */
    if (latest1 == 0)
        return 1;

    latest2 = commit_latest(catalogpath2, 0);

    /* The directory is likely not a valid repo */
    if (latest2 == 0)
        return 1;

    /* Load the hash trees */
    tree1 = commit_loadtree(catalogpath1, latest1);
    tree2 = commit_loadtree(catalogpath2, latest2);

    if (tree1 == NULL || tree2 == NULL)
        return 1;

    /* Are the trees the same? */
    if (!strcmp(tree1->hash, tree2->hash)) {
        printf("The trees are identical.\n");
        return 0;
    }

    return 0;
}
