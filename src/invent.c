#include <stdlib.h>

#include "actor.h"
#include "invent.h"

/* Allocate memory for the item component of a given actor. */
void init_item(struct actor *actor) {
    struct item *new_item = (struct item *) malloc(sizeof(struct item));
    new_item->parent = actor;
    new_item->letter = 'a';
    new_item->quan = 1;
    actor->item = new_item;
}