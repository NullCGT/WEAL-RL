#include <stdlib.h>

#include "map.h"
#include "register.h"
#include "render.h"
#include "windows.h"

/* Render the map, tile by tile.
   Loops over the entirety of the map, and works in O(n) time. */
void render_map(void) {
    clear_map();
    g.cx = min(max(0, g.player.x - (MAPWIN_W  / 2)), abs(MAPW - MAPWIN_W));
    g.cy = min(max(0, g.player.y - (MAPWIN_H / 2)), abs(MAPH - MAPWIN_H));
    for (int i = 0; i < MAPWIN_W; i++) {
        for (int j = 0; j < MAPWIN_H; j++) {
            if (i + g.cx < MAPW && j + g.cy < MAPH
                && i + g.cx >= 0 && j + g.cy >= 0) {
                if (is_visible(i + g.cx, j + g.cy)) {
                    map_putch(i, j, g.levmap[i + g.cx][j + g.cy].chr, WHITE);
                } else if (is_explored(i + g.cx, j + g.cy)) {
                    map_putch(i, j, g.levmap[i + g.cx][j + g.cy].chr, BLUE);
                }
            } else
                map_putch(i, j, ' ', WHITE);  
        }
    }
    f.update_map = 0;
    return;
}

void render_all_npcs(void) {
    struct actor *cur = &g.player;
    while (cur != NULL && is_visible(cur->x, cur->y)) {
        map_putch(cur->x - g.cx, cur->y - g.cy, cur->chr, GREEN);
        cur = cur->next;
    }
    return;
}

void clear_npcs(void) {
    struct actor *cur = &g.player;
    while (cur != NULL && is_visible(cur->x, cur->y)) {
        map_putch(cur->x - g.cx, cur->y - g.cy, g.levmap[cur->x][cur->y].chr, WHITE);
        cur = cur->next;
    }
    return;
}