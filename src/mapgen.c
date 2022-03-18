#include "register.h"

/* Create the level */
void make_level(void) {
    for (int i = 0; i < MAP_W; i++) {
        for (int j = 0; j < MAP_H; j++) {
            if (i < 10 || i > 40 || j < 10 || j > 40) {
                g.levmap[i][j].chr = '#';
                g.levmap[i][j].blocked = 1;
            } else {
                g.levmap[i][j].chr = '.';
                g.levmap[i][j].blocked = 0;
            }
        }
    }
    return; 
}