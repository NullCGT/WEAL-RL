#include "map.h"
#include"mapgen.h"
#include "register.h"
#include "message.h"

/* Make a point on the map visible. */
int make_visible(int x, int y) {
    g.levmap[x][y].visible = 1;
    g.levmap[x][y].explored = 1;
    if (is_opaque(x, y))
        return 1;
    return 0;
}

/* Explores the entire map. */
void magic_mapping(void) {
    for (int y = 0; y < MAPH; y++) {
        for (int x = 0; x < MAPW; x++) {
            g.levmap[x][y].explored = 1;
        }
    }
    logma(MAGENTA, "I punch in the debug function that lets me see the entire map.");
    f.update_map = 1;
}

/* Change the depth via ascending or descending. */
int change_depth(int change) {
    if (g.depth + change < 0) {
        logm("I look up. It's too cloudy to make out any stars.");
        return 0;
    }
    g.depth += change;
    make_level();
    push_actor(g.player, g.player->x, g.player->y);
    return 50;
}

/* Create a heatmap */
void create_heatmap(void) {
    int y, x, y1, x1;
    int changed = 1;
    /* Setup */
    for (y = 0; y < MAPH; y++) {
        for (x = 0; x < MAPW; x++) {
            /* Non-goals */
            if (is_blocked(x, y)) {
                g.levmap[x][y].player_heat = IMPASSABLE;
                g.levmap[x][y].explore_heat = IMPASSABLE;
            } else {
                g.levmap[x][y].player_heat = MAX_HEAT;
                g.levmap[x][y].explore_heat = MAX_HEAT;
            }

            if (g.player->x == x && g.player->y == y)
                g.levmap[x][y].player_heat = 0;
            if (!g.levmap[x][y].explored)
                g.levmap[x][y].explore_heat = 0;
        }
    }
    /* Iterate until nothing has been changed */
    while (changed) {
        changed = 0;
        for (y = 0; y < MAPH; y++) {
            for (x = 0; x < MAPW; x++) {
                for (x1 = x - 1; x1 <= x + 1; x1++) {
                    if (x1 < 0 || x1 >= MAPW) continue;
                    for (y1 = y - 1; y1 <= y + 1; y1++) {
                        if (y1 < 0 || y1 >= MAPH) continue;
                        if (x1 == 0 && y1 == 0) continue;
                        if (g.levmap[x1][y1].player_heat == IMPASSABLE) continue;
                        /* Heatmap updates */
                        if (g.levmap[x1][y1].player_heat > g.levmap[x][y].player_heat + 1) {
                            g.levmap[x1][y1].player_heat = g.levmap[x][y].player_heat + 1;
                            changed = 1;
                        }
                        if (g.levmap[x1][y1].explore_heat > g.levmap[x][y].explore_heat + 1) {
                            g.levmap[x1][y1].explore_heat = g.levmap[x][y].explore_heat + 1;
                            changed = 1;
                        }
                    }
                }
            }
        }
    }
}
