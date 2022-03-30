#include <stdlib.h>
#include <string.h>

#include "register.h"
#include "windows.h"
#include "render.h"
#include "message.h"
#include "action.h"
#include "BearLibTerminal.h"

#define MAP_LAYER 0
#define ACTOR_LAYER 1
#define MSG_LAYER 2
#define SB_LAYER 3
#define POPUP_LAYER 4

static color_t colors[] = {
    0xFF000000, // Black
    0xFFFF0000, // Red
    0xFF008000, // Green
    0xFFFFFF00, // Yellow
    0xFF0000FF, // Blue
    0xFFFF00FF, // Magenta
    0xFF00FFFF, // Cyan
    0xFFFFFFFF  // White
};

void setup_screen(void) {
    terminal_open();
    terminal_set("window: title='WEAL', size=80x40, resizable=true; font: FSEX302.ttf, size=18");
    terminal_layer(MAP_LAYER);
    terminal_color(colors[WHITE]);
}

void cleanup_screen(void) {
    terminal_close();
}

/* TODO: Implement. */
void create_popup_win(const char *title, const char *msg) {
    return;
}

/* TODO: Implement. */
void display_energy_win(void) {
    return;
}

void draw_msg_window(int h, int full) {
    int i = 0;
    int y = 0;
    dimensions_t dim;
    struct msg *cur_msg;
    struct msg *prev_msg;

    if (full) {
        terminal_clear_area(0, MSG_Y, MSG_W, h);
    }
    terminal_layer(MSG_LAYER);
    terminal_clear_area(0, MSG_Y, MSG_W, h);
    cur_msg = g.msg_list;
    while (cur_msg !=  NULL) {
        if (i >= MAX_BACKSCROLL) {
            prev_msg->next = NULL;
            prev_msg = cur_msg;
            cur_msg = cur_msg->next;
            // TODO: Do not handle memory in screen-related files.
            free_msg(prev_msg);
            i++;
            continue;
        } else if (y > h - 2) {
            prev_msg = cur_msg;
            cur_msg = cur_msg->next;
            i++;
            continue;
        }
        terminal_color(colors[cur_msg->attr]);
        dim = terminal_print_ext(0, y, MSG_W, MSG_H, TK_ALIGN_LEFT, cur_msg->msg);
        terminal_color(colors[WHITE]);
        y += dim.height;
        prev_msg = cur_msg;
        cur_msg = cur_msg->next;
        i++;
    }
    terminal_refresh();
    if (full) {
        terminal_read();
        terminal_clear_area(0, MAPWIN_Y, MAPWIN_W, h);
        terminal_refresh();
    }
    terminal_layer(MAP_LAYER);
    f.update_map = 1;
    f.update_msg = 0;
}

int map_putch(int x, int y, int chr, int color) {
    terminal_color(colors[color]);
    terminal_put(x, y + MAPWIN_Y, chr);
    terminal_color(colors[WHITE]);
    return 0;
}

void clear_map(void) {
    terminal_clear_area(0, MAPWIN_Y, MAPWIN_W, MAPWIN_H);
}

void refresh_map(void) {
    terminal_refresh();
}

/* Handle key inputs. Blocking. */
int handle_keys(void) {
    int keycode = terminal_read();
    int shift = 0;
    if (terminal_check(TK_SHIFT)) {
        shift = 1;
    }
    if (keycode == TK_H || keycode == TK_LEFT) {
        return A_WEST;
    } else if (keycode == TK_J || keycode == TK_DOWN) {
        return A_SOUTH;
    } else if (keycode == TK_K || keycode == TK_UP) {
        return A_NORTH;
    } else if (keycode == TK_L || keycode == TK_RIGHT) {
        return A_EAST;
    } else if (keycode == TK_Y) {
        return A_NORTHWEST;
    } else if (keycode == TK_U) {
        return A_NORTHEAST;
    } else if (keycode == TK_N) {
        return A_SOUTHEAST;
    } else if (keycode == TK_B) {
        return A_SOUTHWEST;
    } else if (keycode == TK_PERIOD) {
        return A_REST;
    } else if (keycode == TK_P) {
        return A_FULLSCREEN;
    } else if (keycode == TK_PERIOD && shift) {
        return A_DESCEND;
    } else if (keycode == TK_COMMA && shift) {
        return A_ASCEND;
    } else if (keycode == TK_Q && shift) {
        return A_QUIT;
    } else if (keycode == TK_Z) {
        return A_DEBUG_MAGICMAP;
    }
    return A_NONE;
}