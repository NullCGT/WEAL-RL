#include <stdlib.h>
#include <stdio.h>

#include "register.h"
#include "action.h"
#include "message.h"
#include "map.h"
#include "windows.h"
#include "render.h"
#include "gameover.h"

int is_player(struct actor *);
int autoexplore(void);
int look_down(void);
int pick_up(void);

int is_player(struct actor* mon) {
    return (mon == &g.player);
}

int move_mon(struct actor* mon, int x, int y) {
    int nx = mon->x + x;
    int ny = mon->y + y;
    if ((nx < 0 || ny < 0|| nx >= MAPW || ny >= MAPH)
        && is_player(mon)) {
            logm("I'm not leaving until I find Kate.");
            f.mode_run = 0;
            return 1;
    } else if (is_blocked(nx, ny) && is_player(mon)) {
        if (is_player(mon)) {
            logm("I press my hand to the wall. The concrete is cold.");
            f.mode_run = 0;
        }
	    return 1;
    }
    g.turns++;
    mon->x = nx;
    mon->y = ny;
    /* For testing energy */ 
    mon->energy -= 1;
    if (mon->energy < 0) {
        mon->energy = mon->emax;
    }
    /* This is just temporary. In the future, we can cut down on this
       to prevent excessive map updates. */
    if (is_player(mon)) {
        f.update_map = 1;
        f.update_fov = 1;
    }
    return 0;
}

int look_down() {
    logm("I glance down. I am standing on %s.", g.levmap[g.player.x][g.player.y].pt->name);
    return 0;
}

int pick_up() {
    logm("I brush the %s beneath me with my fingers. There is nothing there to pick up.",
         g.levmap[g.player.x][g.player.y].pt->name);
    return 1;
}

int autoexplore(void) {
    int lx, ly;
    int lowest = MAX_HEAT;
    // Do things
    for (int x = -1; x <= 1; x++) {
        if (x + g.player.x < 0 || x + g.player.x >= MAPW) continue;
        for (int y = -1; y <= 1; y++) {
            if (!x && !y) continue;
            if (y + g.player.y < 0 || y + g.player.y >= MAPH) continue;
            if (g.levmap[x + g.player.x][y + g.player.y].explore_heat <= lowest) {
                lowest = g.levmap[x + g.player.x][y + g.player.y].explore_heat;
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
        move_mon(&g.player, lx, ly);
        return 0;
    } else {
        logm("I don't think there's anywhere else I can explore from here.");
        f.mode_explore = 0;
        return 1;
    }
}

int get_action(void) {
    /* If we are in runmode or are exploring, don't block input. */
    /* TODO: Fix the kludge happening here and make autoexplore return an
       action. */
    if (f.mode_explore) {
        autoexplore();
        return A_NONE;
    }
    /* If running, move in the previously input direction. */
    if (f.mode_run) {
        return g.prev_action;
    }
    /* Otherwise, block input all day :) */
    return handle_keys();
}

void execute_action(int actnum) {
    if (actnum) g.prev_action = actnum;
    switch(actnum) {
        case A_WEST:
            move_mon(&g.player, -1, 0);
            break;
        case A_SOUTH:
            move_mon(&g.player, 0, 1);
            break;
        case A_NORTH:
            move_mon(&g.player, 0, -1);
            break;
        case A_EAST:
            move_mon(&g.player, 1, 0);
            break;
        case A_NORTHWEST:
            move_mon(&g.player, -1, -1);
            break;
        case A_NORTHEAST:
            move_mon(&g.player, 1, -1);
            break;
        case A_SOUTHEAST:
            move_mon(&g.player, 1, 1);
            break;
        case A_SOUTHWEST:
            move_mon(&g.player, -1, 1);
            break;
        case A_REST:
            move_mon(&g.player, 0, 0);
            break;
        case A_ASCEND:
            change_depth(-1);
            break;
        case A_DESCEND:
            change_depth(1);
            break;
        case A_LOOK_DOWN:
            look_down();
            break;
        case A_PICK_UP:
            pick_up();
            break;
        case A_FULLSCREEN:
            draw_msg_window(term.h, 1);
            break;
        case A_EXPLORE:
            autoexplore();
            break;
        case A_HELP:
            display_file_text("data/text/help.txt");
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
}