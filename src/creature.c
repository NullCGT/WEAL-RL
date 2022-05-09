#include <stdlib.h>
#include <string.h>

#include "ai.h"
#include "creature.h"
#include "map.h"
#include "register.h"
#include "random.h"

void mod_attributes(struct actor *);
void mod_ai(struct ai *);

struct permcreature permcreatures[] = {
    PERMCREATURES
};

/* Initialize an actor's attributes from a permcreature struct. */
void init_creature(struct actor *actor, int cindex) {
    actor->chr = permcreatures[cindex].chr;
    actor->tile_offset = permcreatures[cindex].tile_offset;
    actor->color = permcreatures[cindex].color;
    actor->weight = permcreatures[cindex].weight;
    actor->hp = permcreatures[cindex].base_hp;
    actor->hpmax = actor->hp;
    actor->cindex = cindex;
    /* Stuff that isn't done yet */
    actor->invent = NULL;
    actor->item = NULL;
    actor->ai = NULL;
    actor->energy = 0;
    /* Bitfields */
    actor->given_name = 0;
    actor->unique = 0;
    /* Copy AI */
    #if 0
    if (&permcreatures[cindex].ai)
        memcpy(actor->ai, &permcreatures[cindex].ai, sizeof(struct ai));
    #endif
    /* Individual Variance */
    mod_attributes(actor);
    #if 0
    mod_ai(actor->ai);
    #endif
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
    actor->hp += rndmx(g.depth);
}

/* Assign some slight variance to the ai. */
void mod_ai(struct ai *ai) {
    if (!ai)
        return;
    ai->seektime += rndmx(3);
}