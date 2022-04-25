#include "ai.h"
#include "action.h"
#include "register.h"
#include "render.h"
#include "map.h"

#include <stdlib.h>

void take_turn(struct actor *actor) {
    int cost;
    int action = A_NONE;
    struct ai *ai = actor->ai;
    
    /* Refill energy */
    actor->energy += 100;
    /* Increment turn counter */
    if (actor == &g.player) {
        g.turns++;
        render_all();
    }

    while (actor->energy > 0) {
        if (actor == &g.player && !ai) {
            /* Player input */
            action = get_action();
        } else {
            /* AI Decision-Making */
            int lx, ly;
            int lowest = MAX_HEAT;
            for (int x = -1; x <= 1; x++) {
                if (x + actor->x < 0 || x + actor->x >= MAPW) continue;
                for (int y = -1; y <= 1; y++) {
                    if (!x && !y) continue;
                    if (y + actor->y < 0 || y + actor->y >= MAPH) continue;
                    if (g.levmap[x + actor->x][y + actor->y].player_heat <= lowest) {
                        lowest = g.levmap[x + actor->x][y + actor->y].player_heat;
                        lx = x;
                        ly = y;
                    }
                }
            }
            action = dir_to_action(lx, ly);
        }
        cost = execute_action(actor, action);
        actor->energy -= cost;
        actor_sanity_checks(actor);
        if (f.update_fov)
            create_heatmap(); /* VERY EXPENSIVE. */
        render_all();
    }
}