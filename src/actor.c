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
#include "creature.h"
#include "ai.h"

/* Pushes an actor to a new location and updates the levmap
   accordingly. */
void push_actor(struct actor *actor, int dx, int dy) {
    /* TODO: WHY do we have rendering code here???/ */
    if (is_visible(actor->x, actor->y))
        map_put_tile(actor->x - g.cx, actor->y - g.cy, actor->x, actor->y, 
                     g.levmap[actor->x][actor->y].pt->color);
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
}

/* Removes an actor from both the map and the linked list
   of actors. Does not free memory associated with the
   actor. */
struct actor *remove_actor(struct actor *actor) {
    struct actor *cur = g.player;
    struct actor *prev = NULL;
    g.levmap[actor->x][actor->y].actor = NULL;
    while (cur != NULL) {
        if (cur == actor) {
            if (prev != NULL) prev->next = cur->next;
            else prev = NULL;
            return actor;
        }
        prev = cur;
        cur = cur->next;
        /* TODO: WHY do we have rendering code here???/ */
        if (is_visible(actor->x, actor->y))
            map_put_tile(actor->x - g.cx, actor->y - g.cy, actor->x, actor->y, 
                        g.levmap[actor->x][actor->y].pt->color);
    }
    logma(MAGENTA, "Attempting to remove actor that is not there?");
    return actor;
}

/* Perform sanity checks to ensure that any sort of redundant
   data remains in sync. */
void actor_sanity_checks(struct actor *actor) {
    if (g.levmap[actor->x][actor->y].actor != actor) {
        logma(MAGENTA, "Sanity check fail: Actor claims to be at (%d, %d), but is not there.",
              actor->x, actor->y);
    }
    if (actor->cindex < 0) {
        logma(MAGENTA, "Sanity check fail: Actor with bad cindex %d.", actor->cindex);
    }
}

/* Frees an actor and all data that is associated with it. */
void free_actor(struct actor *actor) {
    if (actor->name)
        free(actor->name);
    if (actor->invent)
        free_invent(actor->invent);
    if (actor->ai)
        free(actor->ai);
    if (actor->item)
        free(actor->item);
    free(actor);
}

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

/* Return the name of the actor. */
char *actor_name(struct actor *actor, unsigned flags) {
    /* Increase the namebuffer index. */
    nbi = (nbi + 1) % 4;
    
    /* Reset the name buffer. */
    memset(namebuf[nbi], 0, 64);
    const char *actname;
    if (actor->name->given_name[0] != '\0') actname = actor->name->given_name;
    else actname = actor->name->real_name;

    if ((flags & NAME_THE) && !actor->unique) {
        sprintf(namebuf[nbi], "the %s", actname);
    } else if ((flags & NAME_MY) && !actor->unique) {
        sprintf(namebuf[nbi], "my %s", actname);
    } else if ((flags & NAME_A) && !actor->unique) {
        sprintf(namebuf[nbi], "%s %s", !vowel(namebuf[nbi][0]) ? "a" : "an", actname);
    } else {
        sprintf(namebuf[nbi], "%s", actname);
    }

    if (flags & NAME_CAP) {
        namebuf[nbi][0] = namebuf[nbi][0] - 32;
    }
    return namebuf[nbi];
}