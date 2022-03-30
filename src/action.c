#include <stdlib.h>

#include "register.h"
#include "action.h"
#include "message.h"
#include "map.h"
#include "windows.h"

int is_player(struct npc *);

int is_player(struct npc* mon) {
    return (mon == &g.player);
}

int move_mon(struct npc* mon, int x, int y) {
    int nx = mon->x + x;
    int ny = mon->y + y;
    if (nx < 0 || ny < 0|| nx >= MAPW || ny >= MAPH
        || is_blocked(nx, ny)) {
        if (is_player(mon)) {
            logm("I cannot go that way.");
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

void execute_action(int actnum) {
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
        case A_FULLSCREEN:
            draw_msg_window(MAPWIN_H + MSG_H, 1);
            break;
        case A_QUIT:
            exit(0);
            break;
        case A_DEBUG_MAGICMAP:
            magic_mapping();
            break;
        case A_NONE:
            break;
        default:
            logm("Unrecognized action %d?", actnum);
            break;
    }
}