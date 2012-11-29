#include <stdio.h>
#include <stdlib.h>

#include <shared/cleanup.h>

void cleanup_register(void* element, void (*function)(void *))
{
	static struct destroyList {
		void* element;
        void (*function)(void *);
		struct destroyList *next;
	} *list = NULL, *tmp;
	
	if (element != NULL) {
		tmp = malloc(sizeof(struct destroyList)); /* TODO */
		tmp->next = list;
		tmp->element = element;
        tmp->function = function;
		list = tmp;
		return;
	}
	
	while (list != NULL) {
		list->function(list->element);
		tmp = list;
		list = list->next;
		free(tmp);
	}
}

void cleanup_execute(void)
{
    cleanup_register(NULL, NULL);
}
