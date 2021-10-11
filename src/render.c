

#include "register.h"
#include "windows.h"

/* Render the map, tile by tile.
   Loops over the entirety of the map, and works in O(n) time. */
void render_map(void) {
    for (int i = 0; i < MAP_WIDTH; i++) {
        for (int j = 0; j < MAP_HEIGHT; j++) {
            map_putch(j, i, g.levmap[i][j].chr);    
        }
    }
    f.update_map = 0;
    return;
}

void render_all_monsters(void) {
    struct monster *cur = &g.player;
    while (cur != NULL) {
        map_putch(cur->y, cur->x, cur->chr);
        cur = cur->next;
    }
    return;
}

void clear_monsters(void) {
    struct monster *cur = &g.player;
    while (cur != NULL) {
        map_putch(cur->y, cur->x, g.levmap[cur->x][cur->y].chr);
        cur = cur->next;
    }
    return;
}