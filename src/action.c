#include <stdlib.h>
#include <stdio.h>

#include "register.h"
#include "action.h"
#include "message.h"
#include "map.h"
#include "windows.h"
#include "render.h"
#include "gameover.h"
#include "save.h"

int is_player(struct actor *);
int autoexplore(void);
int look_down(void);
int pick_up(void);

int is_player(struct actor* mon) {
    return (mon == g.player);
}

int move_mon(struct actor* mon, int x, int y) {
    struct actor *target;
    int nx = mon->x + x;
    int ny = mon->y + y;
    if (!in_bounds(nx, ny)) {
        if (is_player(mon)) {
            logm("I'm not leaving until I find Kate.");
            f.mode_run = 0;
            return 0;
        } else {
            return 100;
        }
    } else if (is_blocked(nx, ny)) {
        if (is_player(mon)) {
            logm("I press my hand to the wall. The concrete is cold.");
            f.mode_run = 0;
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
    /* Perform movement */
    push_actor(mon, nx, ny);
    /* This is just temporary. In the future, we can cut down on this
       to prevent excessive map updates. */
    if (is_player(mon)) {
        f.update_map = 1;
        f.update_fov = 1;
    }
    return 100;
}

int look_down() {
    logm("I glance down. I am standing on %s.", g.levmap[g.player->x][g.player->y].pt->name);
    return 0;
}

int pick_up() {
    logm("I brush the %s beneath me with my fingers. There is nothing there to pick up.",
         g.levmap[g.player->x][g.player->y].pt->name);
    return 0;
}

int autoexplore(void) {
    int lx, ly;
    int lowest = MAX_HEAT;
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
    if (lowest < MAX_HEAT) {
        if (!f.mode_explore) {
            logma(YELLOW, "I begin cautiously exploring the area.");
            f.mode_explore = 1;
        }
        return dir_to_action(lx, ly);
    } else {
        logm("I don't think there's anywhere else I can explore from here.");
        f.mode_explore = 0;
        return 0;
    }
}

int get_action(void) {
    /* If we are in runmode or are exploring, don't block input. */
    if (f.mode_explore) {
        return autoexplore();
    }
    /* If running, move in the previously input direction. */
    if (f.mode_run) {
        return g.prev_action;
    }
    /* Otherwise, block input all day :) */
    return handle_keys();
}

static int dir_array[3][3] = {
    { A_NORTHWEST, A_NORTH, A_NORTHEAST },
    { A_WEST,      A_REST,  A_EAST },
    { A_SOUTHWEST, A_SOUTH, A_SOUTHEAST }
};

/* Given a relative coordinate movement, return a direction. */
int dir_to_action(int x, int y) {
    return dir_array[y + 1][x + 1];
}

/* Executes an action and returns the cost in energy. */
int execute_action(struct actor *actor, int actnum) {
    int ret = 0;
    if (actnum) g.prev_action = actnum;
    if (actnum == A_EXPLORE) {
        actnum = autoexplore();
    }
    switch(actnum) {
        case A_WEST:
            ret = move_mon(actor, -1, 0);
            break;
        case A_SOUTH:
            ret = move_mon(actor, 0, 1);
            break;
        case A_NORTH:
            ret = move_mon(actor, 0, -1);
            break;
        case A_EAST:
            ret = move_mon(actor, 1, 0);
            break;
        case A_NORTHWEST:
            ret = move_mon(actor, -1, -1);
            break;
        case A_NORTHEAST:
            ret = move_mon(actor, 1, -1);
            break;
        case A_SOUTHEAST:
            ret = move_mon(actor, 1, 1);
            break;
        case A_SOUTHWEST:
            ret = move_mon(actor, -1, 1);
            break;
        case A_REST:
            ret = move_mon(actor, 0, 0);
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
            ret = pick_up();
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
            end_game();
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