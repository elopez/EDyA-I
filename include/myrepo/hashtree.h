#ifndef __H_HASHTREE__
#define __H_HASHTREE__

#define DEFAULT_MAX_CHILDREN 50
#define DEFAULT_MAX_DIFF 50
#define HASHTREE_DELIM ((HashTreeNode*)0xDEADBEEF)

typedef struct HashTreeNode {
    const char *name;
    const char *hash;
    struct HashTreeNode** children;
} HashTreeNode;

HashTreeNode* hashtree_new(void);
HashTreeNode* hashtree_new_node(const char* name);
HashTreeNode* hashtree_load(const char* name);
void hashtree_append_child(HashTreeNode* parent, HashTreeNode* child);
void hashtree_insert(HashTreeNode *tree, char* path, char* hash);
char *hashtree_compute(HashTreeNode* tree);
void hashtree_print(HashTreeNode* tree, FILE* stream);
const char* hashtree_fetch(HashTreeNode* tree, const char* path);
const char** hashtree_compare(HashTreeNode* old, HashTreeNode* new);
void hashtree_destroy(HashTreeNode* tree);
void hashtree_register_destroy(HashTreeNode* tree);

#define IS_VALID_CHILD(tree, i)  ((tree)->children[(i)] != NULL && \
    (tree)->children[(i)] != HASHTREE_DELIM)
#define IS_LEAF(tree, i)  ((tree)->children[(i)]->children[0] == NULL)

#endif
