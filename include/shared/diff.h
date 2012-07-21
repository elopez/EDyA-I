#include <stdio.h>

#ifndef __H_DIFF__
#define __H_DIFF__

#define OP_INSERT (1 << 0)
#define OP_DELETE (1 << 1)

#define DIFF_OK     (1 << 0)
#define DIFF_ERROR  (1 << 1)
#define DIFF_SAME   (1 << 2)

struct rule {
    struct rule *previous;
    unsigned int operation;
    unsigned int aline;
    unsigned int bline;
};

int diff_lines(struct rule **, char **, unsigned int, char **, unsigned int);

void diff_print(FILE*, struct rule *, char **, char **);

#endif
