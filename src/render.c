#include <stdlib.h>

#include "map.h"
#include "register.h"
#include "render.h"
#include "windows.h"

/* Render the map, tile by tile.
   Loops over the entirety of the map, and works in O(n) time. */
void render_map(void) {
    werase(g.map_win);
    g.cx = min(max(0, g.player.x - (MAPWIN_W  / 2)), abs(MAPW - MAPWIN_W));
    g.cy = min(max(0, g.player.y - (MAPWIN_H / 2)), abs(MAPH - MAPWIN_H));
    for (int i = 0; i < MAPWIN_W; i++) {
        for (int j = 0; j < MAPWIN_H; j++) {
            if (i + g.cx < MAPW && j + g.cy < MAPH
                && i + g.cx >= 0 && j + g.cy >= 0) {
                if (is_visible(i + g.cx, j + g.cy)) {
                    map_putch(j, i, g.levmap[i + g.cx][j + g.cy].chr, COLOR_PAIR(WHITE));
                } else if (is_explored(i + g.cx, j + g.cy)) {
                    map_putch(j, i, g.levmap[i + g.cx][j + g.cy].chr, COLOR_PAIR(BLUE));
                }
            } else
                map_putch(j, i, ' ', COLOR_PAIR(WHITE));  
        }
    }
    f.update_map = 0;
    return;
}

void render_all_npcs(void) {
    struct npc *cur = &g.player;
    while (cur != NULL) {
        map_putch(cur->y - g.cy, cur->x - g.cx, cur->chr, COLOR_PAIR(WHITE));
        cur = cur->next;
    }
    return;
}

void clear_npcs(void) {
    struct npc *cur = &g.player;
    while (cur != NULL) {
        map_putch(cur->y - g.cy, cur->x - g.cx, g.levmap[cur->x][cur->y].chr, COLOR_PAIR(WHITE));
        cur = cur->next;
    }
    return;
}