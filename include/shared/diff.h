#include <stdio.h>

#ifndef __H_DIFF__
#define __H_DIFF__

/* Operations for rules */
#define OP_INSERT (1 << 0)
#define OP_DELETE (1 << 1)

/* Return codes for diff_lines */
#define DIFF_OK     (1 << 0)
#define DIFF_ERROR  (1 << 1)
#define DIFF_SAME   (1 << 2)

struct rule {
    struct rule *previous;
    struct rule *_free;
    unsigned int operation;
    unsigned int aline;
    unsigned int bline;
};

/**
 * Generates a set of rules to convert a file into another
 * @param[out]  rules       On success, the generated rules will be stored here
 * @param[in]   file1       The first file contents
 * @param[in]   len1        The first file length
 * @param[in]   file2       The second file contents
 * @param[in]   len2        The second file length
 * @return DIFF_OK if it all went well
 *         DIFF_SAME if the files are identical
 *         DIFF_ERROR if an error occured
 */
int diff_lines(struct rule **rules, char **file1, unsigned int len1,
               char **file2, unsigned int len2);

/**
 * Prints the diff in standard text format to fp
 * @param[in]   fp          Where to print it to
 * @param[in]   rules       The set of rules to use for printing
 * @param[in]   file1       The first file that was used to generate the rules
 * @param[in]   file2       The second file that was used to generate the rules
 */
void diff_print(FILE * fp, struct rule *rules, char **file1, char **file2);

/**
 * Frees a chain of rules
 * @param[in]   rules       The chain of rules to free
 */
void diff_free_rules(struct rule *rules);

#endif
