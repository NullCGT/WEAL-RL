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
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            g.levmap[x][y].explored = 1;
        }
    }
}

/* Change the depth via ascending or descending. */
void change_depth(int change) {
    if (g.depth + change < 0) {
        logm("You stare up at the sky, and wonder if you will ever see it again.");
        return;
    }
    g.depth += change;
    make_level();
}