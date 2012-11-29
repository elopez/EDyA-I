#include <assert.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>

#include <myrepo/hash.h>
#include <myrepo/sha1.h>

char* hash_file(char *path)
{
    blk_SHA_CTX ctx;
    FILE* file;
    char buffer[4096];
    char *hash;
    void *bhash;
    size_t len;

    /* path should never be null */
    assert(path != NULL);

    file = fopen(path, "r");
    if (file == NULL)
        return NULL;

    /* get some memory to store the hex and bin representation */
    hash = (char *) malloc(41 * sizeof(char));
    if (hash == NULL)
    {
        fclose(file);
        return NULL;
    }

    bhash = malloc(20 * sizeof(unsigned char));
    if (bhash == NULL)
    {
        fclose(file);
        free(hash);
        return NULL;
    }

    /* initialize */
    blk_SHA1_Init(&ctx);

    /* feed hash function */
    while(!feof(file))
    {
        len = fread(buffer, sizeof(char), sizeof(buffer)/sizeof(char), file);
        blk_SHA1_Update(&ctx, buffer, len);
    }

    /* finalize hash */
    blk_SHA1_Final((unsigned char *)bhash, &ctx);

    /* generate hex representation. binary hash is *big endian* */
    #define PIECE(i) (ntohl(*(unsigned int*)((unsigned char*)bhash + (i)*sizeof(unsigned int))))
    sprintf(hash, "%08x%08x%08x%08x%08x", PIECE(0), PIECE(1), PIECE(2),
        PIECE(3), PIECE(4));

	fclose(file);
    free(bhash);
    return hash;
}
