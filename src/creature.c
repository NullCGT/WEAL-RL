#include <stdlib.h>
#include <string.h>

#include "ai.h"
#include "creature.h"
#include "map.h"
#include "register.h"
#include "random.h"
#include "actor.h"
#include "message.h"

void mod_attributes(struct actor *);
void mod_ai(struct ai *);

struct actor permcreatures[] = {
    PERMCREATURES
};

static const char *permnames[] = {
    "human",
    "glassworm"
};

void init_creature(struct actor *actor, int cindex) {
    memcpy(actor, &permcreatures[cindex], sizeof(struct actor));
    mod_attributes(actor);
    /* Set up actor name struct. */
    actor->name = (struct name *) malloc(sizeof(struct name));
    memset(actor->name->given_name, 0, MAXNAMESIZ);
    memset(actor->name->real_name, 0, MAXNAMESIZ);
    strcpy(actor->name->real_name, permnames[cindex]);
    actor->name->given_name[0] = '\0';
}

/* Spawn a creature-like actor. Passing in coordinates that are not in bounds
   will spawn the creature in a random location. */
struct actor *spawn_creature(int cindex, int x, int y) {
    struct actor *cur_actor = g.player;
    struct actor *prev_actor = cur_actor;

    struct actor *actor = (struct actor *) malloc(sizeof(struct actor));
    init_creature(actor, cindex);

    /* Add the creature the list of actors. */
    while (cur_actor != NULL) {
        prev_actor = cur_actor;
        cur_actor = cur_actor->next;
    }
    if (prev_actor == NULL) {
        prev_actor = actor;
    } else {
        prev_actor->next = actor;
    }
    actor->next = NULL;

    /* Spawn at a given location. */
    if (!in_bounds(x, y)) {
        struct coord c = rand_open_coord();
        x = c.x;
        y = c.y;
    }
    actor->x = x;
    actor->y = y;
    push_actor(actor, x, y);

    /* Return the creature */
    return actor;
}

/* Assign some variance to the creature's attributes. */
void mod_attributes(struct actor *actor) {
    actor->weight += min(actor->weight * 2, rndmx(g.depth * 10));
    actor->hpmax += rndmx(g.depth);
    actor->hp = actor->hpmax;
}

/* Assign some slight variance to the ai. */
void mod_ai(struct ai *ai) {
    if (!ai)
        return;
    ai->seektime += rndmx(3);
}