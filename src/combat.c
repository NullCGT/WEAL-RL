/**
 * @file combat.c
 * @author yKestrel (kestrelg@kestrelscry.com)
 * @brief Combat-related functionality.
 * @version 1.0
 * @date 2022-05-26
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "actor.h"
#include "ai.h"
#include "combat.h"
#include "register.h"
#include "gameover.h"
#include "message.h"
#include "random.h"

/**
 * @brief Perform an attack.
 * 
 * @param aggressor The actor initiating the attack.
 * @param target The target of the attack.
 * @return int The cost in energy of making the attack.
 */
int do_attack(struct actor *aggressor, struct actor *target) {
    struct attack attack = choose_attack(aggressor, target); /* While, we can just use the first attack. */
    int damage = d(attack.dam_n, attack.dam_d);
    int bonus = weak_res(attack.dtype, target->resist, target->weak);
    int cost = 100;
    int color = WHITE;
    /* Feedback color */
    if (bonus < 0)
        color = (target == g.player) ? GREEN : RED;
    else if (bonus > 0)
        color = (target == g.player) ? RED : GREEN;
    /* Feedback */
    if (aggressor == g.player) {
        logma(color, "I thump %s for %d damage%s", actor_name(target, NAME_THE), damage,
                (bonus > 0) ? "! Vulnerable!" : (bonus < 0) ? ". Weak..." : ".");
    } else if (target == g.player) {
        logma(color, "%s thumps me for %d damage%s", actor_name(aggressor, NAME_THE | NAME_CAP), damage,
                (bonus > 0) ? "! Vulnerable!" : (bonus < 0) ? ". Weak..." : ".");
    } else {
        logm("%s thumps %s%s", actor_name(aggressor, NAME_THE | NAME_CAP), actor_name(target, NAME_THE),
                (bonus > 0) ? "!" : (bonus < 0) ? "..." : ".");
    }
    /* Apply damage */
    target->hp -= damage;
    if (target == g.player && target->hp <= 0) {
        g.killer = aggressor;
        logm("It's all over...");
        end_game(0);
    } else if (target != g.player && target->hp <= 0) {
        logm("%s dies.", actor_name(target, NAME_THE | NAME_CAP));
        remove_actor(target);
        free_actor(target);
    }
    /* Calculate cost of attack. */
    if (bonus > 0) {
        cost = 50;
    } else if (bonus < 0) {
        cost = 200;
    }
    return cost;
}

/* Checks the weaknesses and resistances of a monster against an incoming damage
   type.
   - resist types > weak types = resist.
   - Weak types > resist types = weak.
   - resist types == weak types = normal */
/**
 * @brief Checks the weaknesses and resistances of a monster against an incoming damage
   type.
 * 
 * @param dtype Bitfield denoting types of incoming damage.
 * @param resist Bitfield denoting types of damage resisted.
 * @param weak Bitfield denoting types of damage one is weak to.
 * @return int The weakness and resistance staus.
- resist types > weak types = resist = -1
- Weak types > resist types = weak = 0
- resist types == weak types = normal = 1
 */
int weak_res(unsigned short dtype, unsigned short resist, unsigned short weak) {
    int blocks = __builtin_popcount(dtype & resist);
    int hits = __builtin_popcount(dtype & weak);

    #define R_RESIST -1
    #define R_NORMAL 0
    #define R_VULN 1

    if (blocks > hits) return R_RESIST;
    if (blocks < hits) return R_VULN;
    return R_NORMAL;
}