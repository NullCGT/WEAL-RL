#include "register.h"
#include "random.h"

/* Create the level */
void make_level(void) {
    /* Initialize the map */
    for (int i = 0; i < MAP_W; i++) {
        for (int j = 0; j < MAP_H; j++) {
            if (rndmx(100) < 45)
                g.levmap[i][j].blocked = 1;
        }
    }

    /* Cellular automata */
    for (int loops = 0; loops < 7; loops++) {
        for (int i = 0; i < MAP_W; i++) {
            for (int j = 0; j < MAP_H; j++) {
                /* Count living neighbors */
                int living = 0;
                for (int x = i - 1; x <= i + 1; x++) {
                    for (int y = j - 1; y <= j + 1; y++) {
                        if (x < 0 || y < 0 || x >= MAP_W || y >= MAP_H)
                            continue;
                        if (x == i && y == j)
                            continue;
                        living += g.levmap[x][y].blocked;
                    }
                }
                /* Kill or revive the cell */
                if (g.levmap[i][j].blocked && (living < 2 || living > 3)) {
                    g.levmap[i][j].blocked = 0;
                } else if (!g.levmap[i][j].blocked && living == 3) {
                    g.levmap[i][j].blocked = 1;
                }


            }
        }
    }

    /* Clean up the map characters */
    for (int i = 0; i < MAP_W; i++) {
        for (int j = 0; j < MAP_H; j++) {
            if (g.levmap[i][j].blocked) g.levmap[i][j].chr = ACS_CKBOARD;
            else g.levmap[i][j].chr = '.';
        }
    }
    return; 
}