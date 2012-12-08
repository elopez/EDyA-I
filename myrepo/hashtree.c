#include <assert.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <shared/cleanup.h>
#include <shared/salloc.h>

#include <myrepo/hash.h>
#include <myrepo/hashtree.h>
#include <myrepo/sha1.h>

static void hashtree_destroy(HashTreeNode * tree);

HashTreeNode *hashtree_new(void)
{
    HashTreeNode *tmp = hashtree_new_node(".");
    cleanup_register(tmp, (void (*)(void *))hashtree_destroy);
    return tmp;
}

HashTreeNode *hashtree_new_node(const char *name)
{
    HashTreeNode *tree;

    /* name must not be NULL */
    assert(name != NULL);

    /* Initialize the node */
    tree = scalloc(1, sizeof(HashTreeNode));
    tree->name = strdup(name);  /* TODO */
    tree->children = NULL;
    tree->_childpos = 0;

    return tree;
}

static void hashtree_append_child(HashTreeNode * parent, HashTreeNode * child)
{
    assert(parent != NULL);
    assert(child != NULL);

    if (parent->_childpos % DEFAULT_MAX_CHILDREN == 0) {
        /* Lets bump the memory here */
        parent->children = srealloc(parent->children,
                                    (DEFAULT_MAX_CHILDREN +
                                     parent->_childpos) *
                                    sizeof(HashTreeNode *));
    }

    parent->children[parent->_childpos++] = (HashTreeNode *) child;
}

void hashtree_insert(HashTreeNode * tree, char *path, char *hash)
{
    unsigned int i = 2;         /* skip "./" */
    unsigned int j;
    unsigned int len;
    char found;
    HashTreeNode *current = tree;
    HashTreeNode *child;

    assert(tree != NULL);
    assert(path != NULL);

    len = strlen(path);

    while (i < len) {
        /* stop on the delimiter */
        if (path[i] != '/') {
            i++;
            continue;
        }

        path[i] = '\0';

        for (j = 0, found = 0; IS_VALID_CHILD(current, j); j++) {
            if (strcmp(current->children[j]->name, path) == 0) {
                /* found the path on the tree */
                current = current->children[j];
                found = 1;
                break;
            }
        }

        /* path not found, add it */
        if (!found) {
            child = hashtree_new_node(path);
            hashtree_append_child(current, child);
            current = child;
        }

        path[i] = '/';
        i++;
    }

    /* add the leaf node */
    child = hashtree_new_node(path);
    hashtree_append_child(current, child);

    if (hash == NULL)
        child->hash = hash_file(path);
    else
        child->hash = strdup(hash);
}

#define PIECE(i) (ntohl(*(unsigned int*)((unsigned char*)bhash + (i)*sizeof(unsigned int))))

char *hashtree_compute(HashTreeNode * tree)
{
    unsigned int i = 0;
    char *hash;
    void *bhash;
    blk_SHA_CTX ctx;

    assert(tree != NULL);

    /* precomputed */
    if (tree->hash != NULL)
        return (char *)tree->hash;

    /* space for binary hash */
    bhash = smalloc(20 * sizeof(unsigned char));

    /* get some memory to store the hex and bin representation */
    hash = smalloc(41 * sizeof(char));

    /* initialize */
    blk_SHA1_Init(&ctx);

    for (i = 0; IS_VALID_CHILD(tree, i); i++) {
        if (!IS_LEAF(tree, i))
            tree->children[i]->hash = hashtree_compute(tree->children[i]);

        blk_SHA1_Update(&ctx, tree->children[i]->hash, 40);
    }

    blk_SHA1_Final((unsigned char *)bhash, &ctx);

    /* generate hex representation. binary hash is *big endian* */
    sprintf(hash, "%08x%08x%08x%08x%08x", PIECE(0), PIECE(1), PIECE(2),
            PIECE(3), PIECE(4));

    free(bhash);
    tree->hash = hash;
    return hash;
}

void hashtree_print(HashTreeNode * tree, FILE * stream)
{
    unsigned int i;

    /* stream and tree should never be NULL */
    assert(tree != NULL);
    assert(stream != NULL);

    fprintf(stream, "file=%s\nhash=%s\n", tree->name, tree->hash);

    for (i = 0; IS_VALID_CHILD(tree, i); i++) {
        if (!IS_LEAF(tree, i))
            hashtree_print(tree->children[i], stream);
        else
            fprintf(stream, "file=%s\nhash=%s\n",
                    tree->children[i]->name, tree->children[i]->hash);
    }
}

HashTreeNode *hashtree_load(const char *file)
{
    char line1[1050];
    char line2[1050];
    char *path;
    char *hash;
    FILE *fp;

    fp = fopen(file, "r");
    if (fp == NULL)
        return NULL;

    /* Ignore first two lines */
    fgets(line1, 1049, fp);
    fgets(line2, 1049, fp);

    HashTreeNode *tree = hashtree_new();

    while (!feof(fp)) {
        if (fgets(line1, 1049, fp) == NULL || fgets(line2, 1049, fp) == NULL)
            break;

        /* TODO: check for correctness */
        path = line1 + 5;       /* +5 removes "file=" */
        hash = line2 + 5;       /* +5 removes "hash=" */
        path[strlen(path) - 1] = '\0';
        hash[strlen(hash) - 1] = '\0';

        hashtree_insert(tree, path, hash);
    }

    hashtree_compute(tree);

    /* Go back and fetch "." hash as integrity check */
    rewind(fp);
    fgets(line1, 1049, fp);
    fgets(line2, 1049, fp);
    line2[strlen(line2) - 1] = '\0';

    assert(strcmp(tree->hash, line2 + 5) == 0);

    fclose(fp);

    return tree;
}

/* TODO: Verify! */
const char *hashtree_fetch(HashTreeNode * tree, const char *cpath)
{
    unsigned int i = 2;         /* skip "./" */
    unsigned int j;
    unsigned int len;
    char found = 0;
    HashTreeNode *current = tree;
    char *path = strdup(cpath); /* TODO */

    assert(tree != NULL);
    assert(path != NULL);

    len = strlen(path);
    while (i <= len) {
        /* stop on the delimiter */
        if (path[i] != '/' && path[i] != '\0') {
            i++;
            continue;
        }

        path[i] = '\0';

        for (j = 0, found = 0; IS_VALID_CHILD(current, j); j++) {
            if (strcmp(current->children[j]->name, path) == 0) {
                /* found the path on the tree */
                current = current->children[j];

                /* if we're mid-string, we need to look further */
                if (i != len) {
                    path[i++] = '/';
                    found = 0;
                    break;
                }

                found = 1;
                break;
            }
        }

        /* We got it! (or we finished and we didn't) */
        if (found == 1 || i == len)
            break;
        i++;
    }

    free(path);

    if (found)
        return current->hash;

    return NULL;
}

struct list {
    const char *name;
    struct list *next;
};

static struct list *hashtree_compare_int(HashTreeNode * old, HashTreeNode * new,
                                         struct list *tmp)
{
    unsigned int i;
    const char *hash;
    struct list *tmp2;

    for (i = 0; IS_VALID_CHILD(new, i); i++) {
        if (!IS_LEAF(new, i))
            tmp = hashtree_compare_int(old, new->children[i], tmp);
        else {
            hash = hashtree_fetch(old, new->children[i]->name);
            if (hash == NULL || strcmp(hash, new->children[i]->hash)) {
                tmp2 = smalloc(sizeof(struct list));
                tmp2->name = new->children[i]->name;
                tmp2->next = tmp;
                tmp = tmp2;
            }
        }
    }

    return tmp;
}

const char **hashtree_compare(HashTreeNode * old, HashTreeNode * new)
{
    const char **table, **tableiter;
    struct list *tmp, *tmp2;
    unsigned int size = DEFAULT_MAX_DIFF, count = 0;

    /* old and new trees should never be NULL */
    assert(old != NULL);
    assert(new != NULL);

    /* if tree heads match, it's the same tree */
    if (!strcmp(old->hash, new->hash))
        return NULL;

    tmp = hashtree_compare_int(old, new, NULL);

    /* convert to a plain array */
    table = scalloc(DEFAULT_MAX_DIFF, sizeof(char *));

    tableiter = table;
    for (; tmp != NULL; count++) {
        if (size == count) {
            size += DEFAULT_MAX_DIFF;
            table = srealloc(table, size * sizeof(char *));
            tableiter = table + size - DEFAULT_MAX_DIFF - 1;
        }
        *(tableiter++) = tmp->name;
        tmp2 = tmp;
        tmp = tmp->next;
        free(tmp2);
    }

    *tableiter = NULL;

    return table;
}

static void hashtree_destroy(HashTreeNode * tree)
{
    unsigned int i;

    /* tree should never be NULL */
    assert(tree != NULL);

    for (i = 0; IS_VALID_CHILD(tree, i); i++) {
        if (!IS_LEAF(tree, i)) {
            hashtree_destroy(tree->children[i]);
        } else {
            free((void *)tree->children[i]->name);
            free((void *)tree->children[i]->hash);
            free((void *)tree->children[i]);
        }
    }

    if (i > 0)
        free((void *)tree->children);
    free((void *)tree->name);
    free((void *)tree->hash);
    free((void *)tree);
}
