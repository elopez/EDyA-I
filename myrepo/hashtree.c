#include <assert.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <shared/cleanup.h>

#include <myrepo/hash.h>
#include <myrepo/hashtree.h>
#include <myrepo/sha1.h>

HashTreeNode* hashtree_new(void)
{
    HashTreeNode *tmp = hashtree_new_node(".");
    cleanup_register(tmp, (void (*)(void*)) hashtree_destroy);
    return tmp;
}

HashTreeNode* hashtree_new_node(const char* name)
{
    HashTreeNode* tree = calloc(1, sizeof(HashTreeNode));
    char* ourname;

    if (tree == NULL)
        return NULL;

    /* duplicate name for ourselves */
    assert(name != NULL);
    ourname = malloc((strlen(name) + 1) * sizeof(char));
    if (ourname == NULL)
    {
        free(tree);
        return NULL;
    }
    strcpy(ourname, name);

    tree->name = ourname;
    tree->children = (HashTreeNode**) calloc(DEFAULT_MAX_CHILDREN,
        sizeof(HashTreeNode*));

    if (tree->children == NULL)
    {
        free(ourname);
        free(tree);
        return NULL;
    }

    /* end of array marker */
    tree->children[DEFAULT_MAX_CHILDREN-1] = HASHTREE_DELIM;

    return tree;
}

void hashtree_append_child(HashTreeNode* parent, HashTreeNode* child)
{
    unsigned int i=0;
    unsigned int j;
    void *data;

    assert(parent != NULL);
    assert(child != NULL);

    while (parent->children[i] != NULL)
    {
        if (parent->children[i] != HASHTREE_DELIM)
            i++;
        else {
            /* Lets bump the memory here */
            data = realloc(parent->children, DEFAULT_MAX_CHILDREN+i+1);
            if (data == NULL)
                return; /* TODO: clean up */

            parent->children = (HashTreeNode**) data;

            /* NULLify the new region and mark the end of it */
            for(j=i+1; j < DEFAULT_MAX_CHILDREN+i; j++)
                parent->children[j] = NULL;
            parent->children[DEFAULT_MAX_CHILDREN+i] = HASHTREE_DELIM;
            break;
        }
    }

    /* Now [i] is either a NULL entry or the old HASHTREE_DELIM */
    parent->children[i] = (HashTreeNode*) child;
}


void hashtree_insert(HashTreeNode *tree, char* path, char* hash)
{
    unsigned int i = 2; /* skip "./" */
    unsigned int j;
    unsigned int len;
    char found;
    HashTreeNode* current = tree;
    HashTreeNode* child;
    struct stat st;

    assert(tree != NULL);
    assert(path != NULL);

    len = strlen(path);

    while (i < len)
    {
        /* stop on the delimiter */
        if (path[i] != '/')
        {
            i++;
            continue;
        }

        path[i] = '\0';

        for (j=0,found=0; IS_VALID_CHILD(current, j); j++)
        {
            if (strcmp(current->children[j]->name, path) == 0)
            {
                /* found the path on the tree */
                current = current->children[j];
                found = 1;
                break;
            }
        }

        /* path not found, add it */
        if (!found)
        {
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
        if (stat(path, &st) == 0)
            child->hash = hash_file(path);
        else
            child->hash = strdup("deadda1adeadda1adeadda1adeadda1adeadda1a");
    else
        child->hash = strdup(hash);
}

char *hashtree_compute(HashTreeNode* tree)
{
    unsigned int i = 0;
    char *hash;
    void *bhash;
    blk_SHA_CTX ctx;

    assert(tree != NULL);

    /* precomputed */
    if (tree->hash != NULL)
        return (char*) tree->hash;

    /* space for binary hash */
    bhash = malloc(20 * sizeof(unsigned char));
    if (bhash == NULL)
        return NULL;

    /* get some memory to store the hex and bin representation */
    hash = (char *) malloc(41 * sizeof(char));
    if (hash == NULL)
    {
        free(bhash);
        return NULL;
    }

    /* initialize */
    blk_SHA1_Init(&ctx);

    for (i=0; IS_VALID_CHILD(tree, i); i++)
    {
        if (!IS_LEAF(tree, i))
            tree->children[i]->hash = hashtree_compute(tree->children[i]);

        blk_SHA1_Update(&ctx, tree->children[i]->hash, 40);
    }

    blk_SHA1_Final((unsigned char *)bhash, &ctx);

    /* generate hex representation. binary hash is *big endian* */
    #define PIECE(i) (ntohl(*(unsigned int*)((unsigned char*)bhash + (i)*sizeof(unsigned int))))
    sprintf(hash, "%08x%08x%08x%08x%08x", PIECE(0), PIECE(1), PIECE(2),
        PIECE(3), PIECE(4));

    free(bhash);
    tree->hash = hash;
    return hash;
}

void hashtree_print(HashTreeNode* tree, FILE* stream)
{
    unsigned int i;

    /* stream and tree should never be NULL */
    assert(tree != NULL);
    assert(stream != NULL);

    fprintf(stream, "file=%s\nhash=%s\n", tree->name, tree->hash);

    for (i=0; IS_VALID_CHILD(tree, i); i++)
    {
        if (!IS_LEAF(tree, i))
            hashtree_print(tree->children[i], stream);
        else
            fprintf(stream, "file=%s\nhash=%s\n",
                tree->children[i]->name, tree->children[i]->hash);
    }
}

HashTreeNode* hashtree_load(const char* file)
{
    char line1[1050];
    char line2[1050];
    char *path;
    char *hash;
    FILE* fp;

    fp = fopen(file, "r");
    if (fp == NULL)
        return NULL; /* TODO */

    /* Ignore first two lines */
    fgets(line1, 1049, fp);
    fgets(line2, 1049, fp);

    HashTreeNode* tree = hashtree_new();

    while(!feof(fp))
    {
        if (fgets(line1, 1049, fp) == NULL || fgets(line2, 1049, fp) == NULL)
            break;

        /* TODO: check for correctness */
        path = line1+5; /* +5 removes "file=" */
        hash = line2+5; /* +5 removes "hash=" */
        path[strlen(path)-1] = '\0';
        hash[strlen(hash)-1] = '\0';

        hashtree_insert(tree, path, hash);
    }

    hashtree_compute(tree);

    /* Go back and fetch "." hash as integrity check */
    rewind(fp);
    fgets(line1, 1049, fp);
    fgets(line2, 1049, fp);
    line2[strlen(line2)-1] = '\0';

    assert(strcmp(tree->hash, line2+5) == 0);

    fclose(fp);

    return tree;
}

/* TODO: Verify! */
const char* hashtree_fetch(HashTreeNode* tree, const char* cpath)
{
    unsigned int i = 2; /* skip "./" */
    unsigned int j;
    unsigned int len;
    char found = 0;
    HashTreeNode* current = tree;
    char *path = strdup(cpath); /* TODO */

    assert(path != NULL);

    len = strlen(path);
    while (i <= len)
    {
        /* stop on the delimiter */
        if (path[i] != '/' && path[i] != '\0')
        {
            i++;
            continue;
        }

        path[i] = '\0';

        for (j=0,found=0; IS_VALID_CHILD(current, j); j++)
        {
            if (strcmp(current->children[j]->name, path) == 0)
            {
                /* found the path on the tree */
                current = current->children[j];

                /* if we're mid-string, we need to look further */
                if (i != len)
                {
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
    const char* name;
    struct list *next;
};

static struct list *hashtree_compare_int(HashTreeNode* old, HashTreeNode* new, struct list *tmp)
{
    unsigned int i;
    const char *hash;
    struct list *tmp2;

    for (i=0; IS_VALID_CHILD(new, i); i++)
    {
        if (!IS_LEAF(new, i))
            tmp = hashtree_compare_int(old, new->children[i], tmp);
        else {
            hash = hashtree_fetch(old, new->children[i]->name);
            if (hash == NULL || strcmp(hash, new->children[i]->hash))
            {
                tmp2 = malloc(sizeof(struct list)); /* TODO */
                tmp2->name = new->children[i]->name;
                tmp2->next = tmp;
                tmp = tmp2;
            }
        }
    }

    return tmp;
}

const char** hashtree_compare(HashTreeNode* old, HashTreeNode* new)
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
    table = calloc(DEFAULT_MAX_DIFF, sizeof(char*)); /*TODO*/
    if (table == NULL) {
        fprintf(stderr, "Error allocating memory for table\n");
        return NULL;
    }

    tableiter = table;
    for(; tmp != NULL; count++) {
        if (size == count) {
            size += DEFAULT_MAX_DIFF;
            table = realloc(table, size * sizeof(char*));
            if (table == NULL)
            {
                fprintf(stderr, "Error reallocating memory for table\n");
                return NULL;
            }
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


void hashtree_destroy(HashTreeNode* tree)
{
    unsigned int i;

    /* stream and tree should never be NULL */
    assert(tree != NULL);

    for (i=0; IS_VALID_CHILD(tree, i); i++)
    {
        if (!IS_LEAF(tree, i)) {
            hashtree_destroy(tree->children[i]);
        } else {
            free((void*)tree->children[i]->children);
            free((void*)tree->children[i]->name);
            free((void*)tree->children[i]->hash);
            free((void*)tree->children[i]);
        }
    }

    free((void*)tree->children);
    free((void*)tree->name);
    free((void*)tree->hash);
    free((void*)tree);
}
