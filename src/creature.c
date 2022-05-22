#include <stdlib.h>
#include <string.h>

#include "ai.h"
#include "creature.h"
#include "map.h"
#include "register.h"
#include "random.h"
#include "actor.h"
#include "message.h"
#include "invent.h"

void init_permcreature(struct actor *, int);
void init_permitem(struct actor *, int);
void init_permname(struct actor *, const char *);
struct actor *spawn_actor(int, int, int, int);
void mod_attributes(struct actor *);
void mod_ai(struct ai *);

struct actor permcreatures[] = {
    PERMCREATURES
};

struct actor permitems[] = {
    PERMITEMS
};

static const char *permcreaturenames[] = {
    "human",
    "zombie"
};

static const char *permitemnames[] = {
    "longsword"
};

void init_permcreature(struct actor *actor, int cindex) {
    memcpy(actor, &permcreatures[cindex], sizeof(struct actor));
    mod_attributes(actor);
    init_ai(actor);
    init_permname(actor, permcreaturenames[cindex]);
}

void init_permitem(struct actor *actor, int cindex) {
    memcpy(actor, &permitems[cindex], sizeof(struct actor));
    mod_attributes(actor);
    init_item(actor);
    init_permname(actor, permitemnames[cindex]);
}

void init_permname(struct actor *actor, const char *permname) {
    actor->name = (struct name *) malloc(sizeof(struct name));
    memset(actor->name->given_name, 0, MAXNAMESIZ);
    memset(actor->name->real_name, 0, MAXNAMESIZ);
    strcpy(actor->name->real_name, permname);
    actor->name->given_name[0] = '\0';
}

struct actor *spawn_creature(int cindex, int x, int y) {
    return spawn_actor(cindex, 1, x, y);
}

struct actor *spawn_item(int cindex, int x, int y) {
    return spawn_actor(cindex, 0, x, y);
}

/* Spawn an actor. Passing in coordinates that are not in bounds
   will spawn the actor in a random location. */
struct actor *spawn_actor(int cindex, int creature, int x, int y) {
    struct actor *cur_actor = g.player;
    struct actor *prev_actor = cur_actor;

    struct actor *actor = (struct actor *) malloc(sizeof(struct actor));
    if (creature)
        init_permcreature(actor, cindex);
    else
        init_permitem(actor, cindex);

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

/* Assign some variance to the actor's attributes. */
void mod_attributes(struct actor *actor) {
    actor->weight += rndmx(actor->weight + g.depth);
    actor->hpmax += rndmx(g.depth);
    actor->hp = actor->hpmax;
}

/* Assign some slight variance to the ai. */
void mod_ai(struct ai *ai) {
    if (!ai)
        return;
    ai->seektime += rndmx(3);
}