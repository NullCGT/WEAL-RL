#include <stdlib.h>

#include "actor.h"
#include "invent.h"

/* Allocate memory for the inventory component of a given actor. */
void init_invent(struct actor *actor) {
    struct invent *new_invent = (struct invent *) malloc(sizeof(struct invent));
    new_invent->parent = actor;
    for (int i = 0; i < MAX_INVENT_SIZE; i++) {
        new_invent->items[i] = NULL;
    }
    actor->invent = new_invent;
}

/* Frees an inventory and associated data */
void free_invent(struct invent *invent) {
    for (int i = 0; i < MAX_INVENT_SIZE; i++) {
        if (!invent->items[i])
            break;
        free(invent->items[i]);
    }
    free(invent);
}

/* Allocate memory for the item component of a given actor. */
void init_item(struct actor *actor) {
    struct item *new_item = (struct item *) malloc(sizeof(struct item));
    new_item->parent = actor;
    new_item->prev_letter = 'a';
    new_item->quan = 1;
}