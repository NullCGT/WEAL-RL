#include <stdlib.h>

#include "map.h"
#include "register.h"
#include "render.h"
#include "windows.h"
#include "message.h"

void put_heatmap(int, int);

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
                if (is_explored(i + g.cx, j + g.cy)) {
                    if (g.display_heat)
                        put_heatmap(i, j);
                    else
                        map_putch(i, j, g.levmap[i + g.cx][j + g.cy].chr, 
                                    is_visible(i + g.cx, j +g.cy) ? WHITE : BLUE);
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

/* HEATMAP RENDERING FUNCTIONS BEGIN HERE */
/* TODO: Break out into own file? */
/* TODO: It may be better to create structs for heatmaps. As of right now, this
   code is quite brittle. That's probably fine, since we have a finite number of
   heatmaps and only debug users can toggle them, but I don't like it. */

#define MAX_HEATMAP_DISPLAY 37
const char heatmap[MAX_HEATMAP_DISPLAY] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ?";

/* Viewmode enum */
enum viewmode {
    V_STANDARD,
    V_HM_PLAYER,
    V_HM_EXPLORE
};
#define V_MAX V_HM_EXPLORE

/* Display a heatmap on the main map. For debug purposes only. */
void put_heatmap(int x, int y) {
    int i;
    switch(g.display_heat) {
        case V_HM_EXPLORE:
            i = g.levmap[x +g.cx][y + g.cy].explore_heat;
            break;
        case V_HM_PLAYER:
        default:
            i = g.levmap[x +g.cx][y + g.cy].player_heat;
            break;
    }
    int color = 0xffffff - i * (0xffffff / MAX_HEATMAP_DISPLAY);
    if (i >= MAX_HEATMAP_DISPLAY) {
        map_putch(x, y, g.levmap[x + g.cx][y + g.cy].chr, WHITE);
        return;
    }
    map_putch_truecolor(x, y, heatmap[i], color);
}

int switch_viewmode(void) {
    g.display_heat = g.display_heat + 1;
    if (g.display_heat > V_MAX) g.display_heat = 0;
    f.update_map = 1;
    logma(MAGENTA, "I punch in the debug function that toggles the level heatmap.");
    return 0;
}