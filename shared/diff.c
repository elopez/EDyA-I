#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <shared/salloc.h>
#include <shared/diff.h>

static struct rule *diff_invert_rules(struct rule *list)
{
    struct rule *newlist = NULL;
    struct rule *tmp;

    while (list != NULL) {
        tmp = list->previous;
        list->previous = newlist;
        newlist = list;
        list = tmp;
    }

    return newlist;
}

int diff_lines(struct rule **ruleset, char **alines, unsigned int aqty,
               char **blines, unsigned int bqty)
{
    unsigned int row = 0;
    unsigned int col = 0;
    unsigned int maxqty = (aqty > bqty ? aqty : bqty);
    unsigned int *last_d_pos;
    unsigned int lower;
    unsigned int upper;
    unsigned int d;
    unsigned int k;
    struct rule **rules;
    struct rule *rule = NULL, *tmp;

    /* find out common strings on start */
    while (row < aqty && row < bqty && strcmp(alines[row], blines[row]) == 0)
        row++;

    /* we won't calculate everything in the matrix so we use
     * these to mark the 'top' and 'bottom' diagonals that are
     * worth calculating in
     */
    lower = (row == aqty) ? maxqty + 1 : maxqty - 1;
    upper = (row == bqty) ? maxqty - 1 : maxqty + 1;

    /* Empty files are obviously the same. lower and upper are unsigned
     * so the next check won't catch it (upper overflows) */
    if (aqty == 0 && bqty == 0)
        return DIFF_SAME;

    /* row == aqty && row == bqty */
    if (lower > upper)
        return DIFF_SAME;

    /* Allocate memory */
    last_d_pos = scalloc(2 * maxqty + 1, sizeof(unsigned int));
    rules = scalloc(2 * maxqty + 1, sizeof(struct rule *));

    last_d_pos[maxqty] = row;

    /* for each value of the edit distance */
    for (d = 1; d <= 2 * maxqty; d++) {
        /* for each relevant diagonal */
        for (k = lower; k <= upper; k += 2) {
            tmp = smalloc(sizeof(struct rule));
            tmp->_free = rule;  /* free chain */
            rule = tmp;

            /* Find a d on diagonal k
             * if k == maxqty-d, this is the lowest diagonal with d's
             * so we should move down
             * if k == maxqty+d, this is the highest diagonal with d's
             * and we should move right
             */
            if (k == maxqty - d ||
                (k != maxqty + d && last_d_pos[k + 1] >= last_d_pos[k - 1])) {
                /* Moving down from the last d-1 on diagonal k+1
                 * puts you further along diagonal k than moving
                 * right from the last d-1 on diagonal k-1. */
                row = last_d_pos[k + 1] + 1;
                rule->previous = rules[k + 1];
                rule->operation = OP_DELETE;
            } else {
                /* Move right from the last d-1 on diagonal k-1 */
                row = last_d_pos[k - 1];
                rule->previous = rules[k - 1];
                rule->operation = OP_INSERT;
            }

            /* Calculate the corresponding column */
            col = row + k - maxqty;

            /* Fill and store the rule on the array for future reference */
            rule->aline = row;
            rule->bline = col;
            rules[k] = rule;

            /* Slide down the diagonal */
            while (row < aqty && col < bqty &&
                   strcmp(alines[row], blines[col]) == 0) {
                row++;
                col++;
            }

            /* We have now computed values on diagonal k up to row */
            last_d_pos[k] = row;

            /* this is the bottom right corner, we found the smallest
             * set of rules to go from alines to blines. The algorythm
             * stops here */
            if (row == aqty && col == bqty) {
                *ruleset = diff_invert_rules(rules[k]);
                free(rules);
                free(last_d_pos);
                return DIFF_OK;
            }

            /* range checks. We break out of the loop here because otherwise
             * the next iteration would be working with out of bound values. */
            if (row == aqty) {
                lower = k + 2;
                break;
            }
            if (col == bqty) {
                upper = k - 2;
                break;
            }
        }

        lower--;
        upper++;
    }

    free(rules);
    free(last_d_pos);
    return DIFF_ERROR;
}

void diff_print(FILE * stream, struct rule *rules, char **alines, char **blines)
{
    struct rule *a, *b, *c, *d;
    unsigned int ischange, isbinary;
    char tag[100];

    /* Small, unreliable hack to detect file type */
    isbinary = (*alines && (*alines)[strlen(*alines)-1] != '\n') ||
               (*blines && (*blines)[strlen(*blines)-1] != '\n');

    while (rules != NULL) {
        switch (rules->operation) {
        case OP_DELETE:
            b = rules;
            do {
                a = b;
                b = b->previous;
            } while (b != NULL && b->operation == OP_DELETE &&
                     b->aline == a->aline + 1);
            /* b is now the first op after the block of deletes */

            ischange = (b != NULL && b->operation == OP_INSERT &&
                        b->aline == a->aline);

            c = d = b;
            if (ischange) {
                /* find out how many ones we are changing */
                do {
                    c = d;
                    d = d->previous;
                } while (d != NULL && d->operation == OP_INSERT &&
                         d->bline == c->bline + 1);
                /* c now points to the element after the change block */

                if (b == c)     /* single line */
                    sprintf(tag, "c%d", c->bline);
                else
                    sprintf(tag, "c%d,%d", b->bline, c->bline);
            } else {
                // if (rules == a) /* single line */
                sprintf(tag, "d%d", rules->bline);
                // else /* does this one make sense? */
                //     sprintf(tag, "d%d,%d", rules->bline, a->bline);
            }

            /* single line */
            if (rules == a)
                fprintf(stream, "%d%s\n", rules->aline, tag);
            else
                fprintf(stream, "%d,%d%s\n", rules->aline, a->aline, tag);

            do {
                fprintf(stream, "< %s", alines[rules->aline - 1]);
                rules = rules->previous;
            } while (rules != b);
            /* now rules is the first insert after the deletes */

            /* Binary files have no newlines at the end of lines */
            if (isbinary)
                fprintf(stream, "\n");

            if (ischange) {
                fprintf(stream, "---\n");
                do {
                    fprintf(stream, "> %s", blines[rules->bline - 1]);
                    rules = rules->previous;
                } while (rules != d);
                /* now rules is the first delete after the inserts */
            }
            break;

        case OP_INSERT:
            /* see how many are we adding */
            b = rules;
            do {
                a = b;
                b = b->previous;
            } while (b != NULL && b->operation == OP_INSERT &&
                     b->bline == a->bline + 1);

            if (rules == a)     /* one line */
                sprintf(tag, "a%d", a->bline);
            else
                sprintf(tag, "a%d,%d", rules->bline, a->bline);

            fprintf(stream, "%d%s\n", rules->aline, tag);

            do {
                fprintf(stream, "> %s", blines[rules->bline - 1]);
                rules = rules->previous;
            } while (rules != b);
            /* rules is now the first instruction post-insert */
            break;
        }

    }
}

void diff_free_rules(struct rule *rules)
{
    struct rule *tmp;

    /* There's no point in freeing a null pointer */
    assert(rules != NULL);

    /* We need to reinvert it to find out the first ->_free */
    rules = diff_invert_rules(rules);

    while (rules != NULL) {
        tmp = rules->_free;
        free(rules);
        rules = tmp;
    }
}
