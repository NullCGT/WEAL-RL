/**
 * @file combat.c
 * @author Kestrel (kestrelg@kestrelscry.com)
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
#include "invent.h"

int attack_roll(struct actor *, struct actor *, struct attack *);

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
    int color = (target == g.player ? BRIGHT_YELLOW : WHITE);
    /* Feedback color */
    if (bonus < 0)
        color = (target == g.player) ? GREEN : RED;
    else if (bonus > 0)
        color = (target == g.player) ? RED : GREEN;
    /* Hit or miss */
    if (!attack_roll(aggressor, target, &attack)) {
        if (aggressor == g.player) {
            g.target = target;
            logm("You miss %s.", actor_name(target, NAME_THE));
        } else if (target == g.player) {
            logm("%s misses you.", actor_name(aggressor, NAME_THE | NAME_CAP));
        } else {
            logm("%s misses %s.", actor_name(aggressor, NAME_THE | NAME_CAP), actor_name(target, NAME_THE));
        }
        return cost;
    }
    /* Feedback */
    if (aggressor == g.player) {
        g.target = target;
        logma(color, "You hit %s for %d damage%s", actor_name(target, NAME_THE), damage,
                (bonus > 0) ? "! Vulnerable!" : (bonus < 0) ? ". Weak..." : ".");
    } else if (target == g.player) {
        if (!g.target) g.target = aggressor;
        logma(color, "%s hits you for %d damage%s", actor_name(aggressor, NAME_THE | NAME_CAP), damage,
                (bonus > 0) ? "! Vulnerable!" : (bonus < 0) ? ". Weak..." : ".");
    } else {
        logm("%s hits %s%s", actor_name(aggressor, NAME_THE | NAME_CAP), actor_name(target, NAME_THE),
                (bonus > 0) ? "!" : (bonus < 0) ? "..." : ".");
    }
    /* Apply damage */
    target->hp -= damage;
    if (target == g.player && target->hp <= 0) {
        g.target = aggressor;
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
int weak_res(unsigned long dtype, unsigned long resist, unsigned long weak) {
    int blocks = __builtin_popcount(dtype & resist);
    int hits = __builtin_popcount(dtype & weak);

    #define R_RESIST -1
    #define R_NORMAL 0
    #define R_VULN 1

    if (blocks > hits) return R_RESIST;
    if (blocks < hits) return R_VULN;
    return R_NORMAL;
}

/**
 * @brief Perform an attack roll.
 * 
 * @param aggressor The actor making the attack.
 * @param target The actor defending.
 * @param attack A pointer to the attack being made.
 * @return int Returns 1 if the attack should hit. Otherwise, returns zero.
 */
int attack_roll(struct actor *aggressor, struct actor *target, struct attack *attack) {
    int goal = 0;
    /* TODO: Check status effects and equips on target and aggressor. */
    goal += calculate_accuracy(aggressor, attack);
    goal -= calculate_evasion(target);

    return (rndrng(1, 101) < goal);
}

/**
 * @brief Calculate the evasion percentage of an actor.
 * 
 * @param actor The actor to calculate evasion for.
 * @return int The evasion percentage in integer form.
 */
int calculate_evasion(struct actor *actor) {
    return actor->evasion + actor->temp_evasion;
}

/**
 * @brief Calculate the accuracy percentage of an actor.
 * 
 * @param actor The actor to calculate accuracy for.
 * @param attack The attack the actor is using.
 * @return int The accuracy percentage in integer form.
 */
int calculate_accuracy(struct actor *actor, struct attack *attack) {
    return actor->temp_accuracy + actor->accuracy + attack->accuracy;
}


/**
 * @brief Cycle the player's active attack. Has the potential to loop infinitely if no attacks are available.
 * 
 */
int cycle_active_attack(void) {
    int max = MAX_ATTK * 3; /* Multiply by three because the player can use off-hand, main-hand, or unarmed attacks. */
    struct actor *old_attacker = g.active_attacker;
    int old_index = g.active_attack_index;
    struct attack *new_attack;

    while (1) {
        /* Increment index */
        g.active_attack_index++;
        if (g.active_attack_index >= max)
            g.active_attack_index = 0;
        /* Check attack at index */
        if (g.active_attack_index < MAX_ATTK) {
            if (!is_noatk(g.player->attacks[g.active_attack_index])) {
                g.active_attacker = g.player;
                break;
            }
        } else if (g.active_attack_index < MAX_ATTK * 2) {
            if (EWEP(g.player) && !is_noatk(EWEP(g.player)->attacks[g.active_attack_index - MAX_ATTK])) {
                g.active_attacker = EWEP(g.player);
                break;
            }
        } else if (EOFF(g.player) && !is_noatk(EOFF(g.player)->attacks[g.active_attack_index - (2 * MAX_ATTK)])) {
            g.active_attacker = EOFF(g.player);
            break;
        }
    }
    /* Notify the player with a message. */
    new_attack = get_active_attack();
    if (old_attacker != g.active_attacker && g.active_attacker != g.player) {
        logm("You brandish %s [%d%%](%dd%d).", actor_name(g.active_attacker, NAME_YOUR),
             new_attack->accuracy, new_attack->dam_n, new_attack->dam_d);
    } else if (old_attacker != g.active_attacker) {
        logm("You switch to using unarmed attacks [%d%%](%dd%d).",
             new_attack->accuracy, new_attack->dam_n, new_attack->dam_d);
    } else if (g.active_attacker != g.player) {
        logm("You change weapon styles with your %s.", actor_name(g.active_attacker, 0),
             new_attack->accuracy, new_attack->dam_n, new_attack->dam_d);
    } else if (old_index != g.active_attack_index) {
        logm("You switch stances.");
    } else {
        logm("You only know one way to attack.");
    }
    return 0;
}

/**
 * @brief Get the active attack
 * 
 * @return struct attack* The pointer to the active attack.
 */
struct attack *get_active_attack(void) {
    if (g.active_attack_index < MAX_ATTK) return &g.active_attacker->attacks[g.active_attack_index];
    else if (g.active_attack_index < MAX_ATTK * 2) return &g.active_attacker->attacks[g.active_attack_index - MAX_ATTK];
    else return &g.active_attacker->attacks[g.active_attack_index - (MAX_ATTK * 2)];
}