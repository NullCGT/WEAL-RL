/**
 * @file actor.c
 * @author Kestrel (kestrelg@kestrelscry.com)
 * @brief Functionality relating to the creation, destruction, and
 description of actors.
 * @version 1.0
 * @date 2022-05-26
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "actor.h"
#include "color.h"
#include "invent.h"
#include "map.h"
#include "message.h"
#include "random.h"
#include "register.h"
#include "windows.h"
#include "gameover.h"
#include "spawn.h"
#include "ai.h"
#include "render.h"

/**
 * @brief Pushes an actor to a new location and updates the levmap
 accordingly.
 * 
 * @param actor The actor to be pushed.
 * @param dx The x coordinate the actor is to be pushed to.
 * @param dy The y coordinate the actor is to be pushed to.
 */
void push_actor(struct actor *actor, int dx, int dy) {
    mark_refresh(actor->x, actor->y);
    if (actor->item) {
        g.levmap[actor->x][actor->y].item_actor = NULL;
        actor->x = dx;
        actor->y = dy;
        g.levmap[actor->x][actor->y].item_actor = actor;
    } else {
        g.levmap[actor->x][actor->y].actor = NULL;
        actor->x = dx;
        actor->y = dy;
        g.levmap[actor->x][actor->y].actor = actor;
    }
    mark_refresh(actor->x, actor->y);
}

/**
 * @brief Removes an actor from both the map and the linked list
 of actors. Does not free memory associated with the actor.
 * 
 * @param actor The actor to be removed.
 * @return struct actor* The actor that has been removed.
 */
struct actor *remove_actor(struct actor *actor) {
    struct actor *cur = g.player;
    struct actor *prev = NULL;
    mark_refresh(actor->x, actor->y);
    if (actor->item)
        g.levmap[actor->x][actor->y].item_actor = NULL;
    else
        g.levmap[actor->x][actor->y].actor = NULL;
    while (cur != NULL) {
        if (cur == actor) {
            if (prev != NULL) prev->next = cur->next;
            else prev = NULL;
            actor->next = NULL;
            return actor;
        }
        prev = cur;
        cur = cur->next;
    }
    logma(MAGENTA, "Attempting to remove actor that is not there?");
    return actor;
}

/**
 * @brief Perform sanity checks to ensure that any sort of redundant data
 remains in sync.
 * 
 * @param actor The actor to perform sanity checks upon.
 */
void actor_sanity_checks(struct actor *actor) {
    if (g.levmap[actor->x][actor->y].actor != actor) {
        logma(MAGENTA, "Sanity check fail: Actor claims to be at (%d, %d), but is not there.",
              actor->x, actor->y);
    }
}

/**
 * @brief Frees an actor and all members of the actor struct.
 * 
 * @param actor The actor to be freed.
 */
void free_actor(struct actor *actor) {
    if (actor->name)
        free(actor->name);
    if (actor->invent)
        free_actor_list(actor->invent);
    if (actor->ai)
        free(actor->ai);
    if (actor->item)
        free(actor->item);
    free(actor);
}

/**
 * @brief Frees a contiguous linked list of actors.
 * 
 * @param actor The head of the linked list.
 */
void free_actor_list(struct actor *actor) {
    struct actor *next;
    while (actor != NULL) {
        next = actor->next;
        free_actor(actor);
        actor = next;
    }
}

/* TODO: Remove magic numbers. */
/* Make use of a rotating set of buffers, just like how NetHack does it. */
static int nbi = -1;
static char namebuf[4][64];

/**
 * @brief Return the name of an actor.
 * 
 * @param actor The actor to be named.
 * @param flags Flags that can modify the characteristics of the returned
 string. For a full list, see actor.h.
 * @return char* The name of the actor.
 */
char *actor_name(struct actor *actor, unsigned flags) {
    /* Increase the namebuffer index. */
    nbi = (nbi + 1) % 4;
    
    /* Reset the name buffer. */
    memset(namebuf[nbi], 0, 64);
    const char *actname;
    if (actor->name->given_name[0] != '\0') actname = actor->name->given_name;
    else actname = actor->name->real_name;

    if ((flags & NAME_THE) && !actor->unique) {
        snprintf(namebuf[nbi], sizeof(namebuf[nbi]), "the %s", actname);
    } else if ((flags & NAME_MY) && !actor->unique) {
        snprintf(namebuf[nbi], sizeof(namebuf[nbi]), "my %s", actname);
    } else if ((flags & NAME_A) && !actor->unique) {
        snprintf(namebuf[nbi], sizeof(namebuf[nbi]), "%s %s", !vowel(namebuf[nbi][0]) ? "a" : "an", actname);
    } else {
        snprintf(namebuf[nbi], sizeof(namebuf[nbi]), "%s", actname);
    }

    if (flags & NAME_CAP) {
        namebuf[nbi][0] = namebuf[nbi][0] - 32;
    }
    return namebuf[nbi];
}