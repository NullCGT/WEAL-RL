/**
 * @file map.c
 * @author Kestrel (kestrelg@kestrelscry.com)
 * @brief Functions related to the level map.
 * @version 1.0
 * @date 2022-05-27
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "map.h"
#include"mapgen.h"
#include "register.h"
#include "message.h"
#include "random.h"
#include "gameover.h"

/**
 * @brief Make a point on the map visible and explored.
 * 
 * @param x x coordinate of the location.
 * @param y y coordinate of the location.
 * @return int Denotes whether tile is opaque.
 */
int make_visible(int x, int y) {
    g.levmap[x][y].visible = 1;
    g.levmap[x][y].explored = 1;
    if (is_opaque(x, y))
        return 1;
    return 0;
}

/**
 * @brief Return a random open coordinate on the map.
 * 
 * @return struct coord The open coordinate found.
 */
struct coord rand_open_coord(void) {
    int x, y;

    do {
        x = rndmx(MAPW);
        y = rndmx(MAPH);
    } while (is_blocked(x, y) || g.levmap[x][y].actor);

    struct coord c = {x, y};

    return c;
}

/**
 * @brief Marks every cell in the map as explored.
 * 
 */
void magic_mapping(void) {
    for (int y = 0; y < MAPH; y++) {
        for (int x = 0; x < MAPW; x++) {
            g.levmap[x][y].explored = 1;
        }
    }
    logma(MAGENTA, "I punch in the debug function that lets me see the entire map.");
    f.update_map = 1;
}

/**
 * @brief Climb a set of stairs. Calls change_depth().
 * 
 * @param change The number of levels climbed and direction of the climb. 
 Must be -1 or 1.
 * @return int Cost in energy of changing depth.
 */
int climb(int change) {
    if (g.depth + change < 0) {
        logm("I look up. It's too cloudy to make out any stars.");
        return 0;
    }
    if (change == 1) {
        if (TILE_AT(g.player->x, g.player->y) == T_STAIR_DOWN) {
            logm("I descend deeper into the complex.");
            return change_depth(change);
        } else {
            logm("I need to find a staircase in order to descend.");
            return 0;
        }
    } else if (change == -1) {
        if (TILE_AT(g.player->x, g.player->y) == T_STAIR_UP) {
            logm("I ascend to an unfamiliar level.");
            return change_depth(change);
        } else {
            logm("Unless I'm supposed to literally climb the walls, I need to find some stairs.");
            return 0;
        }
    }
    logma(MAGENTA, "How odd. I appear to be climbing multiple levels?");
    return change_depth(change);
}

/**
 * @brief Change the player's depth.
 * 
 * @param change The number of levels to shift.
 * @return int The cost in energy of climbing.
 */
int change_depth(int change) {
    g.depth += change;
    if (g.depth >= 25) {
        /* A winner is you. */
        end_game(1);
    }
    free_actor_list(g.player->next);
    make_level();
    push_actor(g.player, g.player->x, g.player->y);
    return 50;
}

static struct coord cardinal_dirs[] = {
    { 0, -1 }, { 1, 0 }, { 0, 1 }, { -1, 0 }
};

/**
 * @brief Create the level's heatmap. An extremely expensive function that should
 be called as little as possible.
 * 
 */
void create_heatmap(void) {
    int y, x, y1, x1;
    int changed = 1;
    /* Setup */
    for (y = 0; y < MAPH; y++) {
        for (x = 0; x < MAPW; x++) {
            /* Non-goals */
            if (is_blocked(x, y) && TILE_AT(x, y) != T_DOOR_CLOSED) {
                g.levmap[x][y].player_heat = IMPASSABLE;
                g.levmap[x][y].explore_heat = IMPASSABLE;
                g.levmap[x][y].goal_heat = IMPASSABLE;
                continue;
            } else {
                g.levmap[x][y].player_heat = MAX_HEAT;
                g.levmap[x][y].explore_heat = MAX_HEAT;
                g.levmap[x][y].goal_heat = !is_explored(x, y) ? IMPASSABLE : MAX_HEAT;
            }

            if (g.player->x == x && g.player->y == y)
                g.levmap[x][y].player_heat = 0;
            if (!g.levmap[x][y].explored)
                g.levmap[x][y].explore_heat = 0;
            if (g.goal_x == x && g.goal_y == y)
                g.levmap[x][y].goal_heat = 0;
        }
    }
    /* Iterate until nothing has been changed */
    while (changed) {
        changed = 0;
        for (y = 0; y < MAPH; y++) {
            for (x = 0; x < MAPW; x++) {
                for (int i = 0; i < 4; i++) {
                    struct coord dir = cardinal_dirs[i];
                    x1 = x + dir.x;
                    y1 = y + dir.y;
                    if (!in_bounds(x1, y1)) continue;
                    /* Heatmap updates */
                    if (g.levmap[x1][y1].player_heat > g.levmap[x][y].player_heat + 1
                        && g.levmap[x1][y1].player_heat != IMPASSABLE) {
                        g.levmap[x1][y1].player_heat = g.levmap[x][y].player_heat + 1;
                        changed = 1;
                    }
                    if (f.mode_explore && g.levmap[x1][y1].explore_heat > g.levmap[x][y].explore_heat + 1
                        && g.levmap[x1][y1].explore_heat != IMPASSABLE) {
                        g.levmap[x1][y1].explore_heat = g.levmap[x][y].explore_heat + 1;
                        changed = 1;
                    }
                    if (f.mode_run && g.levmap[x1][y1].goal_heat > g.levmap[x][y].goal_heat + 1
                        && g.levmap[x1][y1].goal_heat != IMPASSABLE) {
                        g.levmap[x1][y1].goal_heat = g.levmap[x][y].goal_heat + 1;
                        changed = 1;
                    }
                }
            }
        }
    }
}
