#ifndef __H_CLEANUP__
#define __H_CLEANUP__

void cleanup_register(void*, void (*)(void *));
void cleanup_execute(void);

#endif
