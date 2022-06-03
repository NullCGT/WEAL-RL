/**
 * @file ai.c
 * @author Kestrel (kestrelg@kestrelscry.com)
 * @brief Functionality associated with AI, as well as the code for
 taking a turn with a given actor.
 * @version 1.0
 * @date 2022-05-26
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <stdlib.h>

#include "ai.h"
#include "action.h"
#include "register.h"
#include "render.h"
#include "map.h"
#include "random.h"
#include "message.h"

void make_aware(struct actor *, struct actor *);
int check_stealth(struct actor *, struct actor *);

/**
 * @brief Initialize an AI struct.
 * 
 * @param actor The parent of the newly initialized ai struct.
 * @return A pointer to the newly-created ai struct.
 */
struct ai *init_ai(struct actor *actor) {
    struct ai *new_ai = (struct ai *) malloc(sizeof(struct ai));
    *new_ai = (struct ai) { 0 };
    new_ai->parent = actor;
    actor->ai = new_ai;
    return actor->ai;
}

/**
 * @brief An actor takes a turn if able. If the actor is the player, then
 pass control to the user. Otherwise, make use of ai functionality.
 * 
 * @param actor The actor who will be taking the turn.
 */
void take_turn(struct actor *actor) {
    int cost;
    int action = A_NONE;

    if (actor != g.player && !actor->ai)
        return;
    
    /* Refill energy */
    actor->energy += 100;
    if (actor->energy > 0 && actor->energy < 100)
        actor->energy = 100;
    /* Increment turn counter */
    if (actor == g.player) {
        g.turns++;
    }

    while (actor->energy > 0) {
        if (actor == g.player) {
            render_all();
            /* Player input */
            action = get_action();
        } else {
            /* Stealth and Visibility */
            if (!is_visible(actor->x, actor->y)) {
                if (actor->ai->seekcur)
                    actor->ai->seekcur--;
            }
            if (!is_aware(actor, g.player))
                check_stealth(actor, g.player);
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
        if (f.update_fov && actor == g.player) {
            do_heatmaps(); /* VERY EXPENSIVE. */
        }
    }
}

/**
 * @brief Randomly pick an attack from among available attacks. Eventually,
 smart monsters will be able to favor attacks that their target is vulnerable
 to.
 * 
 * @param aggressor The actor performing the attack.
 * @param target The target of the attack.
 * @return struct attack The chosen attack.
 */
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

int is_aware(struct actor *aggressor, struct actor *target) {
    if (!aggressor->ai)
        return 0;
    else if (target == g.player)
        return aggressor->ai->seekcur;
    else
        return 1;
}

#define MAX_SPOT_MSG 2
static const char *spot_msgs[MAX_SPOT_MSG] = { "spots", "notices" };

void make_aware(struct actor *aggressor, struct actor *target) {
    if (aggressor == g.player && is_visible(aggressor->x, aggressor->y)) {
        logm("You notice %s.", actor_name(target, NAME_A));
    }
    if (aggressor->ai) {
        logm("%s %s you%s", actor_name(aggressor, NAME_A | NAME_CAP),
            spot_msgs[rndmx(MAX_SPOT_MSG)],
            in_danger(g.player) ? "!" : "." );
        aggressor->ai->seekcur = aggressor->ai->seekdef;
    }
}

int check_stealth(struct actor *aggressor, struct actor *target) {
    (void) aggressor;
    if (is_visible(target->x, target->y) && is_visible(aggressor->x, aggressor->y))
        make_aware(aggressor, target);
    return 0;
}