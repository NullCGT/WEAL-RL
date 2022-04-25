#include "actor.h"
#include "map.h"
#include "message.h"
#include "register.h"
#include "windows.h"

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
    if (aggressor == &g.player)
        logma(YELLOW, "I thump %s.", target->name);
    else if (target == &g.player)
        logma(RED, "%s thumps me.", aggressor->name);
    else
        logm("%s thumps %s.", aggressor->name, target->name);
    return 100;
}