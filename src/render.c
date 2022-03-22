#include <stdlib.h>

#include "register.h"
#include "render.h"

/* Render the map, tile by tile.
   Loops over the entirety of the map, and works in O(n) time. */
void render_map(void) {
    g.cx = min(max(0, g.player.x - (MAPWIN_W  / 2)), abs(MAP_W - MAPWIN_W));
    g.cy = min(max(0, g.player.y - (MAPWIN_H / 2)), abs(MAP_H - MAPWIN_H));
    for (int i = 0; i < MAPWIN_W; i++) {
        for (int j = 0; j < MAPWIN_H; j++) {
            if (i + g.cx < MAP_W && j + g.cy < MAP_H
                && i + g.cx >= 0 && j + g.cy >= 0)
                map_putch(j, i, g.levmap[i + g.cx][j + g.cy].chr);
            else
                map_putch(j, i, ' ');  
        }
    }
    f.update_map = 0;
    return;
}

void render_all_npcs(void) {
    struct npc *cur = &g.player;
    while (cur != NULL) {
        map_putch(cur->y - g.cy, cur->x - g.cx, cur->chr);
        cur = cur->next;
    }
    return;
}

void clear_npcs(void) {
    struct npc *cur = &g.player;
    while (cur != NULL) {
        map_putch(cur->y - g.cy, cur->x - g.cx, g.levmap[cur->x][cur->y].chr);
        cur = cur->next;
    }
    return;
}

/* Outputs a character to the map window. Wrapper for mvwaddch(). */
int map_putch(int y, int x, int chr) {
    return mvwaddch(g.map_win, y, x, chr); 
}

void render_bar(WINDOW* win, int cur, int max, int x, int y,
                int width, int full, int empty) {
    int pips = (int) ((width - 2) * cur / max);
    for (int i = 0; i < width; i++) {
        if (!i) {
            mvwaddch(win, y, x + i, '[');
        } else if (i <= pips) {
            mvwaddch(win, y, x + i, full);
        } else if (i == width - 1) {
            mvwaddch(win, y, x + i, ']');
        } else {
            mvwaddch(win, y, x + i, empty);
        }
    }
    
}