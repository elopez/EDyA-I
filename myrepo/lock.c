#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include <shared/salloc.h>

#include <myrepo/catalog.h>
#include <myrepo/lock.h>

int myrepo_lock(void)
{
    int lockfd;
    char *catalogpath;
    char *lockpath;
    char *lockwarning = "You seem to be executing another instance "
        "of myrepo.\nThe program will now exit.\n";
    char *integritywarning = "WARNING: the last operation you performed with "
        "myrepo failed.\n\nThe repository might be in an undefined state.\n"
        "If you were committing, you should roll back the last unfinished "
        "commit.\nYou can do so by lowering the commit number by one on "
        "repo/.index/revs/latest\nTo continue using myrepo, please remove "
        "repo/.index/integrity.lock\n";
    struct flock lock = {
        .l_type = F_WRLCK,
        .l_whence = SEEK_SET,
        .l_start = 0,
        .l_len = 1,
    };

    /* Open the myrepo global lock file */
    if ((lockfd = open("/tmp/myrepo.lock", O_CREAT | O_RDWR, 0666)) < 0) {
        fprintf(stderr, "%s", lockwarning);
        return 0;
    }

    /* Ask for a write lock on it */
    if (fcntl(lockfd, F_SETLK, &lock) == -1) {
        fprintf(stderr, "%s", lockwarning);
        close(lockfd);
        return 0;
    }

    /* Now only one myrepo process can run at a time */

    /* If we are not inside of a repo we cannot have an integrity check */
    catalogpath = catalog_locate();
    if (catalogpath == NULL)
        return 1;

    /* If integrity check file exists, print a warning and fail to lock */
    lockpath = smalloc((strlen(catalogpath) + 25) * sizeof(char));
    sprintf(lockpath, "%s/.index/integrity.check", catalogpath);
    if (open(lockpath, O_CREAT | O_EXCL | O_RDWR, 0600) < 0) {
        fprintf(stderr, "%s", integritywarning);
        free(lockpath);
        return 0;
    }

    free(lockpath);

    /* We are all set */
    return 1;
}

void myrepo_unlock(void)
{
    char *catalogpath;
    char *lockpath;

    catalogpath = catalog_locate();
    if (catalogpath == NULL)
        return;

    /*
     * We cannot use smalloc because this will be called from atexit
     * and smalloc may call exit.
     * POSIX.1-2001 says that the result of calling exit more than once
     * is undefined.
     */
    lockpath = malloc((strlen(catalogpath) + 25) * sizeof(char));
    if (lockpath == NULL)
        return;

    /* Remove the integrity check flag */
    sprintf(lockpath, "%s/.index/integrity.check", catalogpath);
    unlink(lockpath);
    free(lockpath);
}
