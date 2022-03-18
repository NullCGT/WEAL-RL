

#include "register.h"
#include "render.h"

/* Render the map, tile by tile.
   Loops over the entirety of the map, and works in O(n) time. */
void render_map(void) {
    for (int i = 0; i < MAP_W; i++) {
        for (int j = 0; j < MAP_H; j++) {
            map_putch(j, i, g.levmap[i][j].chr);    
        }
    }
    f.update_map = 0;
    return;
}

void render_all_npcs(void) {
    struct npc *cur = &g.player;
    while (cur != NULL) {
        map_putch(cur->y, cur->x, cur->chr);
        cur = cur->next;
    }
    return;
}

void clear_npcs(void) {
    struct npc *cur = &g.player;
    while (cur != NULL) {
        map_putch(cur->y, cur->x, g.levmap[cur->x][cur->y].chr);
        cur = cur->next;
    }
    return;
}

/* Outputs a character to the map window. Wrapper for mvwaddch(). */
int map_putch(int y, int x, int chr) {
    return mvwaddch(g.map_win, y, x, chr); 
}

void render_bar(WINDOW* win, int cur, int max, int x, int y,
                int width, char full, char empty) {
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