
/**
 * @file spawn.c
 * @author Kestrel (kestrelg@kestrelscry.com)
 * @brief Functionality related to spawning a new actor.
 * @version 1.0
 * @date 2022-05-27
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <stdlib.h>
#include <string.h>

#include "ai.h"
#include "spawn.h"
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
    "longsword",
    "shortsword",
    "dagger"
};

/**
 * @brief Initialize an actor from the permcreatures array.
 * 
 * @param actor Actor to be initialized. Mutated by this function.
 * @param cindex Index of the permcreatures array.
 */
void init_permcreature(struct actor *actor, int cindex) {
    memcpy(actor, &permcreatures[cindex], sizeof(struct actor));
    mod_attributes(actor);
    init_ai(actor);
    init_permname(actor, permcreaturenames[cindex]);
}

/**
 * @brief Initialize an actor from the permitems array.
 * 
 * @param actor Actor to be initialized. Mutated by this function.
 * @param cindex Index of the permitems array.
 */
void init_permitem(struct actor *actor, int cindex) {
    memcpy(actor, &permitems[cindex], sizeof(struct actor));
    mod_attributes(actor);
    init_item(actor);
    init_permname(actor, permitemnames[cindex]);
}

/**
 * @brief Initialize the name struct of an actor.
 * 
 * @param actor The actor to initialize the name for.
 * @param permname The real name of the actor.
 */
void init_permname(struct actor *actor, const char *permname) {
    actor->name = (struct name *) malloc(sizeof(struct name));
    memset(actor->name->given_name, 0, MAXNAMESIZ);
    memset(actor->name->real_name, 0, MAXNAMESIZ);
    strcpy(actor->name->real_name, permname);
    actor->name->given_name[0] = '\0';
}

/**
 * @brief Spawn a creature at a given location.
 * 
 * @param cindex index of the creature to be spawned.
 * @param x x coordinate. If out of bounds, then spawn in a random location.
 * @param y y coordinate. If out of bounds, then spawn in a random location.
 * @return struct actor* Pointer to the actor spawned.
 */
struct actor *spawn_creature(int cindex, int x, int y) {
    return spawn_actor(cindex, 1, x, y);
}

/**
 * @brief Spawn an item at a given location.
 * 
 * @param cindex index of the item to be spawned.
 * @param x x coordinate. If out of bounds, then spawn in a random location.
 * @param y y coordinate. If out of bounds, then spawn in a random location.
 * @return struct actor* Pointer to the actor spawned.
 */
struct actor *spawn_item(int cindex, int x, int y) {
    return spawn_actor(cindex, 0, x, y);
}

/* Spawn an actor. Passing in coordinates that are not in bounds
   will spawn the actor in a random location. */
/**
 * @brief Spawn an actor in a given location.
 * 
 * @param cindex index of the actor to be spawned.
 * @param creature Whether the actor is a creature or not.
 * @param x x coordinate. If out of bounds, then spawn in a random location.
 * @param y y coordinate. If out of bounds, then spawn in a random location.
 * @return struct actor* Pointer to the actor spawned.
 */
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

/**
 * @brief Mutate a given actor's attributes in order to provide some variance.
 * 
 * @param actor The actor to be mutated.
 */
void mod_attributes(struct actor *actor) {
    if (!actor)
        return;
    actor->weight += rndmx(actor->weight + g.depth);
    actor->hpmax += rndmx(g.depth);
    actor->hp = actor->hpmax;
}

/**
 * @brief Mutate an actor's ai in order to provide some slight variance.
 * 
 * @param ai The ai to be mutated.
 */
void mod_ai(struct ai *ai) {
    if (!ai)
        return;
    ai->seektime += rndmx(3);
}