#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <libgen.h>

#include <shared/diff.h>
#include <shared/readfile.h>
#include <shared/pager.h>

#include <myrepo/compare.h>
#include <myrepo/commit.h>
#include <myrepo/hashtree.h>

static void compare_diff(char *catalogpath1, unsigned int rev1,
                        char *catalogpath2, unsigned int rev2, const char *file,
                        FILE * out)
{
    char **file1;
    char **file2;
    unsigned int len1;
    unsigned int len2;
    struct rule *rules;
    int status;

    /* Load both files */
    len1 = commit_file(catalogpath1, rev1, file, &file1);
    len2 = commit_file(catalogpath2, rev2, file, &file2);

    /* And diff them */
    status = diff_lines(&rules, file1, len1, file2, len2);

    /* DIFF_SAME is acceptable if both files are empty (one might not exist) */
    assert(status != DIFF_ERROR);
    assert(status != DIFF_SAME || (len1 == 0 && len2 == 0));

    fprintf(out, "File: %s\n", file);

    if (status != DIFF_SAME) {
        diff_print(out, rules, file1, file2);
        diff_free_rules(rules);
    } else if (commit_file_is_involved(catalogpath2, rev2, file)) {
        fprintf(out, "The file is empty\n");
    } else {
        fprintf(out, "The empty file was removed\n");
    }
    fprintf(out, "\n");

    freereadfile(file1);
    freereadfile(file2);
}

int myrepo_compare(char *catalogpath1, char *catalogpath2)
{
    unsigned int latest1;
    unsigned int latest2;
    HashTreeNode *tree1;
    HashTreeNode *tree2;
    const char **differences1;
    const char **differences2;
    const char **tmp1;
    const char **tmp2;
    FILE *out;

    assert(catalogpath1 != NULL);
    assert(catalogpath2 != NULL);

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

    /* Trees are different */

    /* Open a pager to print the diff to */
    out = pager_init();

    /* Compare one way */
    differences1 = hashtree_compare(tree1, tree2);
    tmp1 = differences1;

    /* Process the differences */
    while (*tmp1 != NULL) {
        compare_diff(catalogpath1, latest1, catalogpath2, latest2, *tmp1, out);
        tmp1++;
    }

    /* Now compare the reverse way, to catch files only on the old repo.
     * Make sure we don't process files twice. */
    differences2 = hashtree_compare(tree2, tree1);
    tmp2 = differences2;

    while (*tmp2 != NULL) {
        tmp1 = differences1;
        while (*tmp1 != NULL && strcmp(*tmp2, *tmp1))
            tmp1++;

        if (*tmp1 == NULL)
            compare_diff(catalogpath1, latest1, catalogpath2, latest2, *tmp2,
                         out);

        tmp2++;
    }

    pager_close(out);
    free(differences1);
    free(differences2);

    return 0;
}
