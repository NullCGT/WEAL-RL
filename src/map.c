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

#include <stdlib.h>
#include "map.h"
#include"mapgen.h"
#include "register.h"
#include "message.h"
#include "random.h"
#include "gameover.h"

int climb(int);

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
 * @return The cost in energy of magic maping (always 0).
 */
int magic_mapping(void) {
    for (int y = 0; y < MAPH; y++) {
        for (int x = 0; x < MAPW; x++) {
            g.levmap[x][y].explored = 1;
        }
    }
    logma(MAGENTA, "You punch in the debug function that lets me see the entire map.");
    f.update_map = 1;
    return 0;
}

/**
 * @brief Ascend one level of the dungeon.
 * 
 * @return int The cost in energy of ascending.
 */
int ascend() {
    return climb(-1);
}

/**
 * @brief Descend one level of the dungeon.
 * 
 * @return int The cost in energy of descending.
 */
int descend() {
    return climb(1);
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
        logm("You look up. It's too cloudy to make out any stars.");
        return 0;
    }
    if (change == 1) {
        if (TILE_AT(g.player->x, g.player->y) == T_STAIR_DOWN) {
            logm("You descend deeper into the complex.");
            return change_depth(change);
        } else {
            logm("You need to find a staircase in order to descend.");
            return 0;
        }
    } else if (change == -1) {
        if (TILE_AT(g.player->x, g.player->y) == T_STAIR_UP) {
            logm("You ascend to an unfamiliar level.");
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
 * @brief Change the player's depth. If this results in a change in maximum depth,
 * then update the player's score.
 * 
 * @param change The number of levels to shift.
 * @return int The cost in energy of climbing.
 */
int change_depth(int change) {
    g.depth += change;
    if (g.depth > g.max_depth) {
        if (in_danger(g.player)) {
            g.score += (1200 * (g.depth - g.max_depth));
        } else {
            g.score += (1000 * (g.depth - g.max_depth));
        }
        g.max_depth = g.depth;
    }
    if (g.depth >= 25) {
        /* A winner is you. */
        end_game(1);
    }
    free_actor_list(g.player->next);
    g.player->next = NULL;
    make_level();
    push_actor(g.player, g.player->x, g.player->y);
    return 50;
}

static struct coord cardinal_dirs[] = {
    { 0, -1 }, { 1, 0 }, { 0, 1 }, { -1, 0 }
};



struct p_node {
    int heat;
    int x;
    int y;
};

void pq_push(struct p_node arr[], int *last, int heat, int x, int y) {
    /* TODO: Potentially insert at beginning rather than end? */
    arr[*last + 1].heat = heat;
    arr[*last + 1].x = x;
    arr[*last + 1].y = y;
    (*last)++;
}

void pq_swap(struct p_node *a, struct p_node *b) {
    struct p_node temp = *a;
    *a = *b;
    *b = temp;
}

struct p_node pq_pop(struct p_node arr[], int *last) {
    struct p_node ret = { 0 };
    int lowest = IMPASSABLE; /* Assuming no negative numbers. */
    int j = 0;
    for (int i = 0; i <= *last; i++) {
        if (arr[i].heat < lowest) {
            lowest = arr[i].heat;
            j = i;
        }
    }
    pq_swap(&arr[j], &ret);
    pq_swap(&arr[j], &arr[*last]);
    (*last)--;
    return ret;
}

/**
 * @brief Create a heatmap.
 * 
 * @param accessor The function used to access the heat variable.
 */
void create_heatmap(int* (* accessor) (int, int)) {
    struct p_node cur;
    int nx, ny;
    int *n_heat;
    unsigned char visited[MAPW][MAPH] = { 0 };
    struct p_node heap[MAPH * MAPW + 1] = { 0 };
    int last = -1;

    /* Populate heap */
    for (int y = 0; y < MAPH; y++) {
        for (int x = 0; x < MAPW; x++) {
            int val = *(accessor(x, y));
            if (val < MAX_HEAT)
                pq_push(heap, &last, val, x, y);
        }
    }

    /* Dijkstra */
    while (last >= 0) {
        cur = pq_pop(heap, &last);
        for (int i = 0; i < 4; i++) {
            /* Loop through neighbors of cur */
            nx = cur.x + cardinal_dirs[i].x;
            ny = cur.y + cardinal_dirs[i].y;
            if (!in_bounds(nx, ny) || visited[nx][ny]) continue;
            n_heat = accessor(nx, ny);
            visited[nx][ny] = 1;
            if (*n_heat == IMPASSABLE) continue;
            if (cur.heat + 1 < *n_heat) {
                *n_heat = cur.heat + 1;
                pq_push(heap, &last, *n_heat, nx, ny);
            }
        }
    }
}

/**
 * @brief Create all necessary heatmaps for the next round.
 * 
 */
void do_heatmaps(void) {
    int y, x;
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

            /* TODO: Create toggles so we only touch the explore and goal maps when necessary */
            if (g.player->x == x && g.player->y == y) {
                g.levmap[x][y].player_heat = 0;
            }
            if (!g.levmap[x][y].explored && f.mode_explore) {
                g.levmap[x][y].explore_heat = 0;
            }
            if (g.goal_x == x && g.goal_y == y && f.mode_run) {
                g.levmap[x][y].goal_heat = 0;
            }
        }
    }
    create_heatmap(get_playerh);
    if (f.mode_run)
        create_heatmap(get_goalh);
    if (f.mode_explore)
        create_heatmap(get_exploreh);
}
