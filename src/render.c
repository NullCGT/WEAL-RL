#include <stdlib.h>

#include "map.h"
#include "register.h"
#include "fov.h"
#include "render.h"
#include "windows.h"
#include "message.h"

void put_heatmap(int, int);
void render_cursor(void);


/* Perform all rendering tasks. Often called from the main loop. */
void render_all(void) {
    if (f.update_msg) {
        draw_msg_window(term.msg_h, 0);
    }
    if (f.update_fov) {
        clear_fov();
        calculate_fov(g.player->x, g.player->y, 7);
    }
    if (f.update_map) {
        render_map();
    }
    render_all_npcs();
    if (f.mode_look) {
        render_cursor();
    }
    display_energy_win();
    refresh_map();
}

/* Render the cursor when in lookmode. */
void render_cursor(void) {
    map_putch(g.cursor_x - g.cx, g.cursor_y - g.cy, 'X', GREEN);
}

/* Render the map, tile by tile.
   Loops over the entirety of the map, and works in O(n) time. */
void render_map(void) {
    clear_map();
    g.cx = min(max(0, g.player->x - (term.mapwin_w  / 2)), abs(MAPW - term.mapwin_w));
    g.cy = min(max(0, g.player->y - (term.mapwin_h / 2)), abs(MAPH - term.mapwin_h));
    for (int i = 0; i < term.mapwin_w; i++) {
        for (int j = 0; j < term.mapwin_h; j++) {
            if (i + g.cx < MAPW && j + g.cy < MAPH
                && i + g.cx >= 0 && j + g.cy >= 0) {
                if (is_explored(i + g.cx, j + g.cy)) {
                    if (g.display_heat)
                        put_heatmap(i, j);
                    else
                        map_put_tile(i, j, i + g.cx, j + g.cy, 
                            is_visible(i + g.cx, j + g.cy) ? g.levmap[i + g.cx][j + g.cy].pt->color : BLUE);
                } else {
                    map_putch(i, j, ' ', WHITE);
                }
            } else
                map_putch(i, j, ' ', WHITE);  
        }
    }
    f.update_map = 0;
    return;
}

void render_all_npcs(void) {
    struct actor *cur = g.player;
    while (cur != NULL) {
        if (is_visible(cur->x, cur->y)) {
            map_put_actor(cur->x - g.cx, cur->y - g.cy, cur, cur->color);
            /* TODO: Handle visibility and runmode in ai.c */
            if (cur != g.player) {
                f.mode_explore = 0;
                f.mode_run = 0;
            }
        }
        cur = cur->next;
    }
    return;
}

void clear_npcs(void) {
    struct actor *cur = g.player;
    while (cur != NULL && is_visible(cur->x, cur->y)) {
        map_put_tile(cur->x - g.cx, cur->y - g.cy, cur->x, cur->y, g.levmap[cur->x][cur->y].pt->color);
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
        map_put_tile(x, y, x + g.cx, y + g.cy, g.levmap[x][y].pt->color);
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