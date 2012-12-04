#ifndef __H_HASHTREE__
#define __H_HASHTREE__

#define DEFAULT_MAX_CHILDREN 50
#define DEFAULT_MAX_DIFF 50

typedef struct HashTreeNode {
    const char *name;
    const char *hash;
    struct HashTreeNode **children;
    unsigned int _childpos;
} HashTreeNode;

/**
 * Creates a new hash tree and sets up auto cleaning for it
 * @return a pointer to the hash tree root
 */
HashTreeNode *hashtree_new(void);

/**
 * Creates a new node for a hash tree. The node is not attached.
 * @param[in]   name        The file name/path this node is linked to.
 *                          The string will be duplicated
 * @return a pointer to the hash tree node
 */
HashTreeNode *hashtree_new_node(const char *name);

/**
 * Loads a hash tree from a file
 * @param[in]   file        The file name/path containing the tree
 * @return a pointer to the hash tree root, or NULL if an error occured.
 */
HashTreeNode *hashtree_load(const char *file);

/**
 * Inserts a file/path into the hash tree, with an optional precomputed
 * hash value.
 * @param[in]   tree        The hash tree to insert the file in
 * @param[in]   path        The file/path to insert
 * @param[in]   hash        (optional) SHA1 hash of path's contents
 */
void hashtree_insert(HashTreeNode * tree, char *path, char *hash);

/**
 * Computes missing hashes on a hash tree.
 * @param[in]   tree        The hash tree to use
 * @return SHA1 hash of the root of the tree
 */
char *hashtree_compute(HashTreeNode * tree);

/**
 * Saves a hash tree to a file. You can then load it again using hashtree_load
 * @param[in]   tree        The hash tree to save
 * @param[in]   stream      The FILE * to save the tree to
 */
void hashtree_print(HashTreeNode * tree, FILE * stream);

/**
 * Fetches the corresponding SHA1 hash of a file/path from a hash tree
 * @param[in]   tree        The hash tree to look in
 * @param[in]   path        The file/path to look for
 * @return the corresponding SHA1 hash, or NULL if it was not found
 */
const char *hashtree_fetch(HashTreeNode * tree, const char *path);

/**
 * Compares two hash trees and returns a NULL-terminated list of file paths
 * where differences were found.
 * @param[in]   old         The "old" hash tree
 * @param[in]   new         The "new" hash tree
 * @return a NULL-terminated list of file paths where differences were found,
 *         or NULL if no differences were found.
 */
const char **hashtree_compare(HashTreeNode * old, HashTreeNode * new);

/**
 * Macro that checks if tree has a valid "(i+1)-th" child
 */
#define IS_VALID_CHILD(tree, i)  ((tree)->children && (i) < (tree)->_childpos)

/**
 * Macro that checks if a valid child is a leaf
 */
#define IS_LEAF(tree, i)  ((tree)->children[(i)]->children == NULL)

#endif
