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
    terminal_set("window: title='WEAL', size=80x40");
    terminal_layer(MAP_LAYER);
    terminal_color(colors[WHITE]);
}

void cleanup_screen(void) {
    terminal_close();
}

void display_file_text(const char *fname) {
    FILE *fp;
    int i, y;
    int j = 0;
    int action = A_NONE;
    char *line = NULL;
    size_t len = 0;

    terminal_clear();
    terminal_layer(POPUP_LAYER);
    
    while (1) {
        i = 0;
        y = 0;
        terminal_clear();
        fp = fopen(fname, "r");
        if (fp == NULL)
            return;
        while (getline(&line, &len, fp) != -1) {
            if (i < j) {
                i++;
                continue;
            }
            terminal_print_ext(0, y++, MSG_W + MAPWIN_W, MSG_H + MAPWIN_H, TK_ALIGN_LEFT, line);
        }
        terminal_refresh();
        fclose(fp);
        action = handle_keys();
        switch (action) {
            case A_NORTH:
            case A_ASCEND:
                j -= 1;
                break;
            case A_SOUTH:
            case A_DESCEND:
                j += 1;
                break;
            case A_QUIT:
            case A_HELP:
                terminal_clear();
                terminal_layer(MAP_LAYER);
                f.update_map = 1;
                f.update_msg = 1;
                return;
        }
        j = max(0, j);
    }
}

/* TODO: Implement. */
void create_popup_win(const char *title, const char *msg) {
    (void) title;
    (void) msg;
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

    if (full) {
        terminal_clear_area(0, MSG_Y, MSG_W, h);
    }
    terminal_layer(MSG_LAYER);
    terminal_clear_area(0, MSG_Y, MSG_W, h);
    cur_msg = g.msg_list;
    while (cur_msg !=  NULL) {
        if (y > h - 2) {
            cur_msg = cur_msg->next;
            i++;
            continue;
        }
        terminal_color(colors[cur_msg->attr]);
        dim = terminal_print_ext(0, y, MSG_W, MSG_H, TK_ALIGN_LEFT, cur_msg->msg);
        terminal_color(colors[WHITE]);
        y += dim.height;
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

int map_put_tile(int x, int y, int mx, int my, int color) {
    terminal_color(colors[color]);
    terminal_put(x, y + MAPWIN_Y, g.levmap[mx][my].pt->chr);
    terminal_color(colors[WHITE]);
    return 0;
}

int map_putch(int x, int y, int chr, int color) {
    terminal_color(colors[color]);
    terminal_put(x, y + MAPWIN_Y, chr);
    terminal_color(colors[WHITE]);
    return 0;
}

int map_putch_truecolor(int x, int y, int chr, unsigned color) {
    terminal_color(color + 0xff000000);
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
    int ret = A_NONE;
    int shift = 0;
    if (terminal_check(TK_SHIFT)) {
        shift = 1;
    }
    if (keycode == TK_H || keycode == TK_LEFT) {
        ret = A_WEST;
    } else if (keycode == TK_J || keycode == TK_DOWN) {
        ret = A_SOUTH;
    } else if (keycode == TK_K || keycode == TK_UP) {
        ret = A_NORTH;
    } else if (keycode == TK_L || keycode == TK_RIGHT) {
        ret = A_EAST;
    } else if (keycode == TK_Y) {
        ret = A_NORTHWEST;
    } else if (keycode == TK_U) {
        ret = A_NORTHEAST;
    } else if (keycode == TK_N) {
        ret = A_SOUTHEAST;
    } else if (keycode == TK_B) {
        ret = A_SOUTHWEST;
    } else if (keycode == TK_PERIOD && shift) {
        ret = A_DESCEND;
    } else if (keycode == TK_COMMA && shift) {
        ret = A_ASCEND;
    } else if (keycode == TK_PERIOD) {
        ret = A_REST;
    } else if (keycode == TK_P) {
        ret = A_FULLSCREEN;
    } else if (keycode == TK_X) {
        ret = A_EXPLORE;
    } else if (keycode == TK_SLASH && shift) {
        ret = A_HELP;
    } else if (keycode == TK_Q && shift) {
        ret = A_QUIT;
    } else if (keycode == TK_R && terminal_check(TK_CONTROL)) {
        ret = A_DEBUG_MAGICMAP;
    } else if (keycode == TK_E && terminal_check(TK_CONTROL)) {
        ret = A_DEBUG_HEAT;
    }
    /* Toggle runmode */
    if (is_movement(ret) && shift) {
        f.mode_run = f.mode_map;
    }
    return ret;
}