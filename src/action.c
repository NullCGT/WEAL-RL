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

int is_player(struct actor *);
int autoexplore(void);
int look_down(void);
int pick_up(void);
int directional_action(const char *, int (* act)(struct actor *, int, int));
struct coord action_to_dir(int);

int is_player(struct actor* mon) {
    return (mon == g.player);
}

int move_mon(struct actor* mon, int x, int y) {
    struct actor *target;
    int nx = mon->x + x;
    int ny = mon->y + y;
    int ret = 0;
    
    /* TEMP */
    f.update_map = 1;

    /* Immediately exit if out of bounds */
    if (!in_bounds(nx, ny)) {
        if (is_player(mon)) {
            logm("I'm not leaving until I find Kate.");
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
    /* Tile-based effects, such as walls and doors. */
    if (g.levmap[nx][ny].pt->func) {
        ret = g.levmap[nx][ny].pt->func(mon, nx, ny);
        if (ret) {
            f.mode_run = 0;
            return ret;
        }
    }
    /* Handle blocked movement */
    if (is_blocked(nx, ny)) {
        if (is_player(mon)) {
            f.mode_run = 0;
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

/* Given a relative coordinate movement, return a direction. */
int dir_to_action(int x, int y) {
    return dir_act_array[y + 1][x + 1];
}

/* Given a movement-based action, return a coordinate. */
struct coord action_to_dir(int actnum) {
    if (!is_movement(actnum)) {
        return act_dir_array[0];
    }
    return act_dir_array[actnum];
}

/* Executes an action and returns the cost in energy. */
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