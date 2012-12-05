#include <stdio.h>
#include <stdlib.h>

#include <shared/salloc.h>
#include <shared/cleanup.h>

void cleanup_register(void *element, void (*function) (void *element))
{
    static struct destroyList {
        void *element;
        void (*function) (void *);
        struct destroyList *next;
    } *list = NULL, *tmp;

    /* Allocate an element and attach it to the destroy list */
    if (element != NULL) {
        tmp = smalloc(sizeof(struct destroyList));
        tmp->next = list;
        tmp->element = element;
        tmp->function = function;
        list = tmp;
        return;
    }

    /* destroy every element on our list */
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
