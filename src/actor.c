#include <stdlib.h>

#include "actor.h"
#include "color.h"
#include "invent.h"
#include "map.h"
#include "message.h"
#include "random.h"
#include "register.h"
#include "windows.h"
#include "gameover.h"

/* Pushes an actor to a new location and updates the levmap
   accordingly. */
void push_actor(struct actor *actor, int dx, int dy) {
    /* TODO: WHY do we have rendering code here???/ */
    if (is_visible(actor->x, actor->y))
        map_put_tile(actor->x - g.cx, actor->y - g.cy, actor->x, actor->y, 
                     g.levmap[actor->x][actor->y].pt->color);
    g.levmap[actor->x][actor->y].actor = NULL;
    actor->x = dx;
    actor->y = dy;
    g.levmap[actor->x][actor->y].actor = actor;
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
}

/* Returns action cost. */
int do_attack(struct actor *aggressor, struct actor *target) {
    int damage = d(1, 6);
    if (aggressor == g.player)
        logma(YELLOW, "I thump %s for %d damage.", target->name, damage);
    else if (target == g.player)
        logma(RED, "%s thumps me for %d damage.", aggressor->name, damage);
    else
        logm("%s thumps %s.", aggressor->name, target->name);
    /* Apply damage */
    target->hp -= damage;
    if (target == g.player && target->hp <= 0) {
        g.killer = aggressor;
        logm("It's all over...");
        end_game();
    } else if (target != g.player && target->hp <= 0) {
        logm("%s dies.", target->name);
        remove_actor(target);
        free_actor(target);
    }
    return 100;
}

/* Frees an actor and all data that is associated with it. */
void free_actor(struct actor *actor) {
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