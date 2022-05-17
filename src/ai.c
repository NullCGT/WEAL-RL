#include "ai.h"
#include "action.h"
#include "register.h"
#include "render.h"
#include "map.h"
#include "random.h"

#include <stdlib.h>

void take_turn(struct actor *actor) {
    int cost;
    int action = A_NONE;
    struct ai *ai = actor->ai;
    
    /* Refill energy */
    actor->energy += 100;
    if (actor->energy > 0 && actor->energy < 100)
        actor->energy = 100;
    /* Increment turn counter */
    if (actor == g.player) {
        g.turns++;
    }

    while (actor->energy > 0) {
        if (actor == g.player && !ai) {
            render_all();
            /* Player input */
            action = get_action();
        } else {
            /* AI Decision-Making */
            int lx, ly = -99;
            int lowest = MAX_HEAT;
            for (int x = -1; x <= 1; x++) {
                if (x + actor->x < 0 || x + actor->x >= MAPW) continue;
                for (int y = -1; y <= 1; y++) {
                    if (!x && !y) continue;
                    /* Hack to prevent monsters eating each other. Will need to be revised in
                      the future. */
                    if (g.levmap[actor->x + x][actor->y + y].actor &&
                        g.levmap[actor->x + x][actor->y + y].actor != g.player) continue;
                    if (y + actor->y < 0 || y + actor->y >= MAPH) continue;
                    if (g.levmap[x + actor->x][y + actor->y].player_heat <= lowest) {
                        lowest = g.levmap[x + actor->x][y + actor->y].player_heat;
                        lx = x;
                        ly = y;
                    }
                }
            }
            if (lx == -99 || ly == -99) {
                action = A_REST;
            } else {
                action = dir_to_action(lx, ly);
            }
        }
        cost = execute_action(actor, action);
        actor->energy -= cost;
        actor_sanity_checks(actor);
        if (f.update_fov && actor == g.player)
            create_heatmap(); /* VERY EXPENSIVE. */
    }
}

/* Randomly pick an attack from among available attacks. Eventually, implement
   smart monsters favoring attacks that the target is vulnerable to. */
struct attack choose_attack(struct actor *aggressor, struct actor *target) {
    int i, j;
    (void) target; /* TODO: Implement attack favoring. */
    for (i = 0; i < MAX_ATTK; i++) {
        if (is_noatk(aggressor->attacks[i]))
            break;
    }
    if (i == 1) return aggressor->attacks[0];
    j = rndmx(i);
    return aggressor->attacks[j];
}