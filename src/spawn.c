
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
#include <stdio.h>

#include "ai.h"
#include "spawn.h"
#include "map.h"
#include "register.h"
#include "random.h"
#include "actor.h"
#include "message.h"
#include "invent.h"
#include "parser.h"

struct actor *spawn_actor(const char *name, int x, int y);
void mod_attributes(struct actor *);
void mod_ai(struct ai *);
void mod_slots(struct item *);

/**
 * @brief Initialize the name struct of an actor.
 * 
 * @param actor The actor to initialize the name for.
 * @param permname The real name of the actor.
 * @return A pointer to the newly-created name struct.
 */
struct name *init_permname(struct actor *actor, const char *permname) {
    actor->name = (struct name *) malloc(sizeof(struct name));
    *actor->name = (struct name) { 0 };
    strcpy(actor->name->real_name, permname);
    actor->name->given_name[0] = '\0';
    return actor->name;
}

/**
 * @brief Spawn a creature at a location. Wrapper for spawn_actor.
 * 
 * @param name The name of the creature to spawn.
 * @param x The x coordinate to spawn at.
 * @param y THe y coordinate to spawn at.
 * @return struct actor* A pointer to the creature spawned.
 */
struct actor *spawn_creature(const char *name, int x, int y) {
    struct actor *actor;
    char buf[64];
    snprintf(buf, sizeof(buf), "data/creature/%s.json", name);
    actor = spawn_actor(buf, x, y);
    return actor;
}

/**
 * @brief Spawn an item at a location. Wrapper for spawn_actor.
 * 
 * @param name The name of the item to spawn.
 * @param x The x coordinate to spawn at.
 * @param y THe y coordinate to spawn at.
 * @return struct actor* A pointer to the item spawned.
 */
struct actor *spawn_item(const char *name, int x, int y) {
    struct actor *actor;
    char buf[64];
    snprintf(buf, sizeof(buf), "data/item/%s.json", name);
    actor = spawn_actor(buf, x, y);
    return actor;
}

struct actor *add_actor_to_main(struct actor *actor) {
    struct actor *cur_actor = g.player;
    struct actor *prev_actor = cur_actor; 

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
    return actor;
}

/**
 * @brief Spawn an actor at a location. If an invalid
 location is passed in, then choose a random one.
 * 
 * @param name The filename of the actor's JSON definition.
 * @param x The x coordinate to spawn at.
 * @param y THe y coordinate to spawn at.
 * @return struct actor* A pointer to the actor spawned.
 */
struct actor *spawn_actor(const char *name, int x, int y) {
    struct actor *actor = actor_from_file(name);

    if (!actor)
        return NULL;

    mod_attributes(actor);
    mod_ai(actor->ai);
    mod_slots(actor->item);

    /* Add the actor the list of actors. */
    add_actor_to_main(actor);

    /* Spawn at a given location. */
    if (!in_bounds(x, y)) {
        struct coord c = rand_open_coord();
        x = c.x;
        y = c.y;
    }
    actor->x = x;
    actor->y = y;
    push_actor(actor, x, y);

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
    actor->hpmax += rndmx(1 + g.depth);
    actor->hp = actor->hpmax;
    for (int i = 0; i < MAX_ATTK; i++) {
        if (is_noatk(actor->attacks[i])) continue;
        actor->attacks[i].accuracy += rndrng(-4, 5);
        actor->attacks[i].dam_d += rndrng(-1, 2);
    }
}

/**
 * @brief Mutate an actor's ai in order to provide some slight variance.
 * 
 * @param ai The ai to be mutated.
 */
void mod_ai(struct ai *ai) {
    if (!ai)
        return;
    ai->seekdef += rndmx(3);
}

/**
 * @brief Set up the item's preferred slots based on its type.
 * 
 * @param item The item to be mutated.
 */
void mod_slots(struct item *item) {
    if (!item)
        return;
    /* The preferred slot is always possible, as are hand slots. */
    item->poss_slot |= slot_types[item->pref_slot].field;
    item->poss_slot |= slot_types[SLOT_WEP].field;
    item->poss_slot |= slot_types[SLOT_OFF].field;
    /* All shields can be worn on the back for minor protection. */
    if (is_shield(item->parent) || is_weapon(item->parent))
        item->poss_slot |= slot_types[SLOT_BACK].field;
    /* Pants can be hats, if you try hard enough. */
    if (is_pants(item->parent))
        item->poss_slot |= slot_types[SLOT_HEAD].field;
    /* And the shirt off your back... */
    if (is_shirt(item->parent))
        item->poss_slot |= slot_types[SLOT_BACK].field;
}