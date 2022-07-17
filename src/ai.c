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
#include "combat.h"

void make_aware(struct actor *, struct actor *);
int check_stealth(struct actor *, struct actor *);
void increment_regular_values(struct actor *);

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
    int cost = 100;
    struct action *action = NULL;

    if (actor != g.player && !actor->ai)
        return;
    
    /* Refill energy */
    actor->energy += 100;
    if (actor->energy > 0 && actor->energy < 100)
        actor->energy = 100;
    increment_regular_values(actor);

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
            /* TODO: Give monsters something to do when unaware of the player. */
            if (is_aware(actor, g.player)) {
                /* AI Decision-Making */
                struct coord cl = best_adjacent_tile(actor->x, actor->y, 1, get_playerh);
                if (cl.x == -99 || cl.y == -99) {
                    action = dir_to_action(0, 0);
                } else {
                    action = dir_to_action(cl.x, cl.y);
                }
            }
        }
        if (action)
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
    if (aggressor == g.player) {
        return *(get_active_attack());
    }
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
        if (!g.target) g.target = aggressor;
    }
}

int check_stealth(struct actor *aggressor, struct actor *target) {
    (void) aggressor;
    if (is_visible(target->x, target->y) && is_visible(aggressor->x, aggressor->y))
        make_aware(aggressor, target);
    return 0;
}

/**
 * @brief Increment and decrement the values that shift every turn an actor takes.
 * 
 * @param actor The actor whose values are to be altered.
 */
void increment_regular_values(struct actor *actor) {
    /* Increment turn counter */
    if (actor == g.player) {
        g.turns++;
        g.score--;
    }
    /* Temporary evasion and accuracy slowly return to zero.  */
    if (actor->temp_accuracy != 0) {
        actor->temp_accuracy < 0 ? actor->temp_accuracy++ : actor->temp_accuracy--;
    }
    if (actor->temp_evasion != 0) {
        actor->temp_evasion < 0 ? actor->temp_evasion++ : actor->temp_evasion--;
    }
}