/**
 * @file action.c
 * @author Kestrle (kestrelg@kestrelscry.com)
 * @brief Contains functionality related to actions. Actions are decisions
 made by the player or other actors which may cost energy.
 * @version 1.0
 * @date 2022-05-26
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "register.h"
#include "action.h"
#include "message.h"
#include "map.h"
#include "windows.h"
#include "render.h"
#include "gameover.h"
#include "save.h"
#include "combat.h"
#include "invent.h"

int is_player(struct actor *);
int autoexplore(void);
int travel(void);
int look_down(void);
int pick_up(struct actor *);
int lookmode(void);
int directional_action(const char *, int (* act)(struct actor *, int, int));
struct coord action_to_dir(int);

/**
 * @brief Determine if a given actor is the player
 * 
 * @param actor The actor to check. 
 * @return int True if the actor passed in is the player.
 */
int is_player(struct actor* actor) {
    return (actor == g.player);
}

/**
 * @brief Moves a creature a relative amount in a given direction.
 * 
 * @param mon The creature to be moved.
 * @param x The number of cells to move in along the x axis.
 * @param y The number of cells to move along the y axis.
 * @return int The cost of the action in energy.
 */
int move_mon(struct actor* mon, int x, int y) {
    struct actor *target;
    int nx = mon->x + x;
    int ny = mon->y + y;
    int ret = 0;

    /* Immediately exit if out of bounds */
    if (!in_bounds(nx, ny)) {
        if (is_player(mon)) {
            logm("I'm not leaving until I find Kate.");
            stop_running();
            return 0;
        } else {
            return 100;
        }
    }
    /* If there is someone there, attack them! */
    target = MON_AT(nx, ny);
    if (target && target != mon) {
        return do_attack(mon, target);
    }
    /* Tile-based effects, such as walls and doors. */
    if (g.levmap[nx][ny].pt->func) {
        ret = g.levmap[nx][ny].pt->func(mon, nx, ny);
        if (ret) {
            stop_running();
            return ret;
        }
    }
    /* Resting costs zero movement */
    if (!x && !y)
        return 100;
    /* Handle blocked movement */
    if (is_blocked(nx, ny)) {
        if (is_player(mon)) {
            stop_running();
            return 0;
        } else {
            return 100;
        }
    }
    /* Perform movement */
    push_actor(mon, nx, ny);
    /* This is just temporary. In the future, we can cut down on this
       to prevent excessive map updates. */
    if (is_player(mon)) {
        f.update_fov = 1;
    }
    return 100;
}

/**
 * @brief Describe the current cell the player is located at.
 * 
 * @return int The cost of the action in energy.
 */
int look_down() {
    if (ITEM_AT(g.player->x, g.player->y)) {
        logm("I glance down. There is %s resting on the %s here.", 
            actor_name(ITEM_AT(g.player->x, g.player->y), NAME_A),
            g.levmap[g.player->x][g.player->y].pt->name);
    } else {
        logm("I glance down. I am standing on %s.", g.levmap[g.player->x][g.player->y].pt->name);
    }
    return 0;
}

/**
 * @brief Pick up an item located at a given creature's location.
 * 
 * @param creature 
 * @return int The cost of picking up an item in energy.
 Failing to pick up an item costs 100 energy.
 Picking up an item costs 50 energy.
 Fumbling an item in an attempt to pick it up costs 50 energy.
 */
int pick_up(struct actor *creature) {
    struct actor *item = ITEM_AT(creature->x, creature->y);
    if (!item) {
        logm("I brush the %s beneath me with my fingers. There is nothing there to pick up.",
            g.levmap[creature->x][creature->y].pt->name);
        return 0;
    }
    /* Remove the actor. If we cannot put it in the inventory, put it back. */
    remove_actor(item);
    if (add_to_invent(creature, item)) {
        logm("I pick up %s [%c].", actor_name(item, NAME_THE), item->item->letter);
        return 50;
    } else {
        push_actor(item, item->x, item->y);
        logm("My bag is too full.");
        return 50;
    }
}

/**
 * @brief Enter into lookmode. Takes in blocking input, wresting control
 of the player away from the user.
 * 
 * @return int The cost in energy. Should always return zero.
 */
int lookmode(void) {
    int act;
    struct coord move_coord;

    f.mode_look = 1;
    g.cursor_x = g.player->x;
    g.cursor_y = g.player->y;
    logm("What should I examine?");
    while (1) {
        f.update_map= 1;
        render_all();
        act = get_action();
        if (is_movement(act)) {
            move_coord = action_to_dir(act);
            g.cursor_x += move_coord.x;
            g.cursor_y += move_coord.y;
        } else if (act == A_QUIT) {
            f.mode_look = 0;
            f.update_map = 1;
            render_all();
            return 0;
        } else if (act == A_LOOK) {
            look_at(g.cursor_x, g.cursor_y);
        }
    }
    return 0;
}

/**
 * @brief Describe a location and the actors at that location.
 * 
 * @param x The x coordinate of the location to be described.
 * @param y The y coordinate of the location to be described.
 * @return int The cost in energy of looking at the location.
 Should always return zero, unless we implement some sort of bizarre monster
 that can steal turns if you examine it.
 */
int look_at(int x, int y) {
    if (!in_bounds(x, y)) {
        logm("There is nothing to see there.");
        return 0;
    } else if (is_visible(x, y)) {
        if (MON_AT(x, y)) {
            g.target = MON_AT(x, y);
            logm("That is %s.", actor_name(MON_AT(x, y), NAME_A));
        } else if (ITEM_AT(x, y)) {
            logm("That is %s.", actor_name(ITEM_AT(x, y), NAME_A));
        } else {
            logm("That is %s %s.", an(g.levmap[x][y].pt->name), g.levmap[x][y].pt->name);
        }
    } else if (is_explored(x, y)) {
        logm("That is %s %s.", an(g.levmap[x][y].pt->name), g.levmap[x][y].pt->name);
    } else {
        logm("I haven't explored that area.");
    }
    return 0;
}

/**
 * @brief Calculates a direction to automatically explore in.
 * 
 * @return int The index of the action that the player will perform.
 */
int autoexplore(void) {
    int lx = IMPASSABLE;
    int ly = IMPASSABLE;
    int lowest = MAX_HEAT;
    
    /* Regenerate the heatmap if exploration is just beginning. */
    if (!f.mode_explore) {
        f.mode_explore = 1;
        do_heatmaps();
    }
    // Do things
    for (int x = -1; x <= 1; x++) {
        if (x + g.player->x < 0 || x + g.player->x >= MAPW) continue;
        for (int y = -1; y <= 1; y++) {
            if (!x && !y) continue;
            if (y + g.player->y < 0 || y + g.player->y >= MAPH) continue;
            if (g.levmap[x + g.player->x][y + g.player->y].explore_heat <= lowest) {
                lowest = g.levmap[x + g.player->x][y + g.player->y].explore_heat;
                lx = x;
                ly = y;
            }
        }
    }
    if (lx == IMPASSABLE || ly == IMPASSABLE) {
        f.mode_explore = 0;
        return A_NONE;
    }
    if (lowest < MAX_HEAT) {
        if (!f.mode_explore) {
            logma(YELLOW, "I begin cautiously exploring the area.");
            f.mode_explore = 1;
        }
        return dir_to_action(lx, ly);
    } else {
        logm("I don't think there's anywhere else I can explore from here.");
        f.mode_explore = 0;
        return A_NONE;
    }
}

/**
 * @brief Calculates the next step when traveling to a specific location.
 * 
 * @return int The index of the action that the player will perform.
 */
int travel(void) {
    int lx = IMPASSABLE;
    int ly = IMPASSABLE;
    int lowest = MAX_HEAT;
    if (g.goal_x == g.player->x && g.goal_y == g.player->y) {
        stop_running();
        return A_NONE;
    }

    // Do things
    for (int x = -1; x <= 1; x++) {
        if (x + g.player->x < 0 || x + g.player->x >= MAPW) continue;
        for (int y = -1; y <= 1; y++) {
            if (!x && !y) continue;
            if (y + g.player->y < 0 || y + g.player->y >= MAPH) continue;
            if (g.levmap[x + g.player->x][y + g.player->y].goal_heat <= lowest) {
                lowest = g.levmap[x + g.player->x][y + g.player->y].goal_heat;
                lx = x;
                ly = y;
            }
        }
    }
    if (lx >= MAX_HEAT || ly == MAX_HEAT) {
        stop_running();
        return A_NONE;
    }
    return dir_to_action(lx, ly);
}

/**
 * @brief Cease all travel-related movement.
 * 
 */
void stop_running(void) {
    f.mode_run = 0;
    g.goal_x = -1;
    g.goal_y = -1;
}

/**
 * @brief Prompt the user for a direction in which to perform an action.
 * 
 * @param actstr A string denoting the action that the player user is preparing to take.
 * @param act The function of the action that will be called.
 * @return int The energy cost of the action that was called.
 */
int directional_action(const char *actstr, int (* act)(struct actor *, int, int)) {
    int actnum;
    struct coord c;
    
    logma(GREEN, "What direction should I %s in?", actstr);
    render_all(); /* TODO: Figure out why this needs render all. */
    while (1) {
        actnum = get_action();
        if (actnum == A_QUIT) {
            logm("Scratch that.");
            return 0;
        } else if (is_movement(actnum))
            break;
    }
    c = action_to_dir(actnum);
    return act(g.player, g.player->x + c.x, g.player->y + c.y);
}

/**
 * @brief Determine the action that the player will be taking. Blocks input.
 * 
 * @return int The cost of the action to be taken.
 */
int get_action(void) {
    /* If we are in runmode or are exploring, don't block input. */
    if (f.mode_explore) {
        return autoexplore();
    }
    /* If running, move towards the goal location if there is one. Otherwise, move 
       in the previously input direction. */
    if (f.mode_run && in_bounds(g.goal_x, g.goal_y) && is_explored(g.goal_x, g.goal_y)) {
        return travel();
    } else if (f.mode_run) {
        return g.prev_action;
    }
    /* Otherwise, block input all day :) */
    return handle_keys();
}

static int dir_act_array[3][3] = {
    { A_NORTHWEST, A_NORTH, A_NORTHEAST },
    { A_WEST,      A_REST,  A_EAST },
    { A_SOUTHWEST, A_SOUTH, A_SOUTHEAST }
};

static struct coord act_dir_array[] = {
    { 0, 0 },
    { -1, 0 },
    { 1, 0 },
    { 0, -1 },
    { 0, 1 },
    { -1, -1 },
    { 1, -1 },
    { -1, 1 }, 
    { 1, 1 }
};

/**
 * @brief Given a relative coordinate movement, return a direction.
 * 
 * @param x The x coordinate: Should fall between -1 and 1 inclusive.
 * @param y The y coordinate. Should fall between -1 and 1 inclusive.
 * @return int The index of the action to be called.
 */
int dir_to_action(int x, int y) {
    return dir_act_array[y + 1][x + 1];
}

/**
 * @brief Given a movement-based action, return the coordinates associated
 with that direction of movement.
 * 
 * @param actnum The index of the action.
 * @return struct coord Coordinate representing the direction of the action.
 */
struct coord action_to_dir(int actnum) {
    if (!is_movement(actnum)) {
        return act_dir_array[0];
    }
    return act_dir_array[actnum];
}

/**
 * @brief Executes an action and returns the cost in energy.
 * 
 * @param actor The actor who will be performing the action.
 * @param actnum The index of the action to be performed.
 * @return int The cost in energy of the action.
 */
int execute_action(struct actor *actor, int actnum) {
    int ret = 0;
    struct coord move_coord;
    if (actnum && actor == g.player) g.prev_action = actnum;
    if (actnum == A_EXPLORE) {
        actnum = autoexplore();
    }
    if (is_movement(actnum)) {
        move_coord = action_to_dir(actnum);
        return move_mon(actor, move_coord.x, move_coord.y);
    }
    switch(actnum) {
        case A_REST:
            ret = move_mon(actor, 0, 0);
            break;
        case A_OPEN:
            ret = directional_action("open", open_door);
            break;
        case A_CLOSE:
            ret = directional_action("close", close_door);
            break;
        case A_LOOK:
            ret = lookmode();
            break;
        case A_ASCEND:
            ret = climb(-1);
            break;
        case A_DESCEND:
            ret = climb(1);
            break;
        case A_LOOK_DOWN:
            ret = look_down();
            break;
        case A_PICK_UP:
            ret = pick_up(g.player);
            break;
        case A_INVENT:
            ret = display_invent();
            break;
        case A_FULLSCREEN:
            draw_msg_window(term.h, 1);
            break;
        case A_HELP:
            display_file_text("data/text/help.txt");
            break;
        case A_SAVE:
            save_exit();
            break;
        case A_QUIT:
            logm("I give up...");
            end_game(0);
            break;
        case A_DEBUG_MAGICMAP:
            magic_mapping();
            break;
        case A_DEBUG_HEAT:
            switch_viewmode();
            break;
        case A_NONE:
            break;
        default:
            logma(RED, "Action %d? I don't understand.", actnum);
            break;
    }
    return ret;
}