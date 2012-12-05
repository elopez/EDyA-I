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

    while(list != NULL)
    {
        tmp = list->previous;
        list->previous = newlist;
        newlist = list;
        list = tmp;
    }

    return newlist;
}

int diff_lines(struct rule ** ruleset, char **alines, unsigned int aqty,
                char **blines, unsigned int bqty)
{
    unsigned int row = 0;
    unsigned int col = 0;
    unsigned int maxqty = (aqty > bqty ? aqty: bqty);
    unsigned int last_d_pos[2*maxqty+1];
    unsigned int lower;
    unsigned int upper;
    unsigned int d;
    unsigned int k;
    struct rule *rules[2*maxqty+1];
    struct rule *rule;

    /* find out common strings on start */
    while(row < aqty && row < bqty && strcmp(alines[row], blines[row]) == 0)
        row++;

    last_d_pos[maxqty] = row;
    rules[maxqty] = NULL;

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

    /* for each value of the edit distance */
    for (d = 1; d <= 2*maxqty; d++)
    {
        /* for each relevant diagonal */
        for (k = lower; k <= upper; k += 2)
        {
            rule = smalloc(sizeof(struct rule));

            /* Find a d on diagonal k */
            if (k == maxqty-d ||
                (k != maxqty+d && last_d_pos[k+1] >= last_d_pos[k-1]))
            {
                /* Moving down from the last d-1 on diagonal k + 1
                 * puts you farther along diagonal k than does
                 * moving right from the last d-1 on diagonal k-1.
                 */
                row = last_d_pos[k+1]+1;
                rule->previous = rules[k+1];
                rule->operation = OP_DELETE;
            } else {
                /* Move right from the last d-1 on diagonal k-1 */
                row = last_d_pos[k-1];
                rule->previous = rules[k-1];
                rule->operation = OP_INSERT;
            }

            /* Code common to the two cases. */
            rule->aline = row;
            rule->bline = col = row + k - maxqty;

            /* TODO: leak? */
            rules[k] = rule;

            /* slide down the diagonal */
            while (row < aqty && col < bqty &&
                   strcmp(alines[row], blines[col]) == 0)
            {
                row++;
                col++;
            }

            last_d_pos[k] = row;

            /* this is the bottom-right corner, we have our answer! */
            if (row == aqty && col == bqty)
            {
                *ruleset = diff_invert_rules(rules[k]);
                return DIFF_OK;
            }

            /* range checks */
            if (row == aqty)
                lower = k+2;
            if (col == bqty)
                upper = k-2;
        }

        lower--;
        upper++;
    }

    return DIFF_ERROR;
}

void diff_print(FILE* stream, struct rule *rules, char **alines, char **blines)
{
    struct rule *a, *b, *c, *d;
    unsigned int ischange;
    char tag[100];
    while (rules != NULL)
    {
        switch (rules->operation)
        {
            case OP_DELETE:
                b = rules;
                do {
                    a = b;
                    b = b->previous;
                } while (b != NULL && b->operation == OP_DELETE &&
                         b->aline == a->aline+1);
                /* b is now the first op after the block of deletes */

                ischange = (b != NULL && b->operation == OP_INSERT &&
                            b->aline == a->aline);

                c = d = b;
                if (ischange)
                {
                    /* find out how many ones we are changing */
                    do {
                        c = d;
                        d = d->previous;
                    } while (d != NULL && d->operation == OP_INSERT &&
                             d->bline == c->bline+1);
                    /* c now points to the element after the change block */

                    if (b == c) /* single line*/
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
                    fprintf(stream, "%d,%d%s\n", rules->aline,
                        a->aline, tag);

                do {
                    fprintf(stream, "< %s", alines[rules->aline-1]);
                    rules = rules->previous;
                } while (rules != b);
                /* now rules is the first insert after the deletes */

                if (ischange)
                {
                    fprintf(stream, "---\n");
                    do {
                        fprintf(stream, "> %s", blines[rules->bline-1]);
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
                         b->bline == a->bline+1);

                if (rules == a) /* one line */
                    sprintf(tag, "a%d", a->bline);
                else
                    sprintf(tag, "a%d,%d", rules->bline, a->bline);

                fprintf(stream, "%d%s\n", rules->aline, tag);

                do {
                    fprintf(stream, "> %s", blines[rules->bline-1]);
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

    while (rules != NULL) {
        tmp = rules->previous;
        free(rules);
        rules = tmp;
    }
}
