#include <stdlib.h>
#include <string.h>

#include "register.h"
#include "windows.h"
#include "render.h"
#include "message.h"
#include "action.h"
#include "map.h"
#include "BearLibTerminal.h"

#define MAP_LAYER 0
#define ACTOR_LAYER 1
#define MSG_LAYER 2
#define SB_LAYER 3
#define POPUP_LAYER 4

#define TILE_WIDTH 16
#define TILE_HEIGHT 16
#define FONT_WIDTH 8
#define FONT_HEIGHT 16
#define WIDTH_MUL (TILE_WIDTH / FONT_WIDTH)
#define HEIGHT_MUL (TILE_HEIGHT / FONT_HEIGHT)

static color_t colors[] = {
    0xFF000000, // Black
    0xFFFF0000, // Red
    0xFF00FF00, // Green
    0xFFFFFF00, // Yellow
    0xFF0000FF, // Blue
    0xFFFF00FF, // Magenta
    0xFF00FFFF, // Cyan
    0xFFFFFFFF  // White
};

int handle_mouse(int);

void setup_screen(void) {
    terminal_open();
    terminal_set("window: title='WEAL', cellsize=8x16, size=90x40, resizeable=true");
    terminal_set("0x1000: tiles.png, size=16x16, resize=16x16, resize-filter=nearest, spacing=2x1");
    terminal_set("0x2000: monsters.png, size=16x16, resize=16x16, resize-filter=nearest, spacing=2x1");
    terminal_set("input: filter='keyboard, mouse'");
    terminal_layer(MAP_LAYER);
    terminal_color(colors[WHITE]);


    setup_term_dimensions(term.h, term.w, HEIGHT_MUL, WIDTH_MUL);
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
    f.mode_map = 0;
    
    while (1) {
        i = 0;
        y = 0;
        terminal_clear();
        fp = fopen(fname, "r");
        if (fp == NULL) {
            f.mode_map = 1;
            return;
        }
        while (getline(&line, &len, fp) != -1) {
            if (i < j) {
                i++;
                continue;
            }
            terminal_print_ext(0, y++, term.w, term.h, TK_ALIGN_LEFT, line);
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
                free(line);
                f.update_map = 1;
                f.update_msg = 1;
                f.mode_map = 1;
                return;
        }
        j = max(0, j);
    }
}

void create_popup_win(const char *title, const char *msg) {
    (void) title;
    (void) msg;
    return;
}

void display_energy_win(void) {
    return;
}

void draw_msg_window(int h, int full) {
    int i = 0;
    int y = 0;
    dimensions_t dim;
    struct msg *cur_msg;

    if (full) {
        terminal_clear();
    }
    terminal_layer(MSG_LAYER);
    terminal_clear_area(0, 0, term.msg_w, h);
    cur_msg = g.msg_list;
    while (cur_msg !=  NULL) {
        if (y > h - 2) {
            cur_msg = cur_msg->next;
            i++;
            continue;
        }
        terminal_color(colors[cur_msg->attr]);
        dim = terminal_print_ext(0, y, term.msg_w, term.msg_h, TK_ALIGN_LEFT, cur_msg->msg);
        terminal_color(colors[WHITE]);
        y += dim.height;
        cur_msg = cur_msg->next;
        i++;
    }
    terminal_refresh();
    if (full) {
        terminal_read();
        terminal_clear();
        terminal_refresh();
    }
    terminal_layer(MAP_LAYER);
    f.update_map = 1;
    f.update_msg = 0;
}

int map_put_tile(int x, int y, int mx, int my, int color) {
    terminal_color(colors[color]);
    terminal_put(x * WIDTH_MUL, (y + term.mapwin_y) * HEIGHT_MUL, 0x1000 + g.levmap[mx][my].pt->id);
    terminal_color(colors[WHITE]);
    return 0;
}

int map_put_actor(int x, int y, struct actor *actor, int color) {
    return map_putch(x, y, actor->tile_offset, color);
}

int map_putch(int x, int y, int chr, int color) {
    terminal_color(colors[color]);
    terminal_put(x * WIDTH_MUL, (y + term.mapwin_y) * HEIGHT_MUL, chr);
    terminal_color(colors[WHITE]);
    return 0;
}

int map_putch_truecolor(int x, int y, int chr, unsigned color) {
    terminal_color(color + 0xff000000);
    terminal_put(x * WIDTH_MUL, (y + term.mapwin_y) * HEIGHT_MUL, chr);
    terminal_color(colors[WHITE]);
    return 0;
}

void clear_map(void) {
    terminal_clear_area(0, term.mapwin_y, term.mapwin_w, term.mapwin_h);
}

void refresh_map(void) {
    terminal_refresh();
}

/* Handle mouse events. Non-blocking. */
int handle_mouse(int event) {
    int x = terminal_state(TK_MOUSE_X);
    int y = terminal_state(TK_MOUSE_Y);
    int gx = (x / WIDTH_MUL) + g.cx;
    int gy = y + g.cy - term.mapwin_y;
    if (f.mode_look) {
        g.cursor_x = gx;
        g.cursor_y = gy;
    }

    if (event == TK_MOUSE_LEFT && in_bounds(gx, gy) && is_explored(gx, gy)
        && !is_blocked(gx, gy)) {
        g.goal_x = gx;
        g.goal_y = gy;
        f.mode_run = 1;
        return A_NONE;
    }
    if (event == TK_MOUSE_RIGHT) {
        look_at(gx, gy);
    }
    return A_NONE;
}

/* Handle key inputs. Blocking. */
int handle_keys(void) {
    int keycode = terminal_read();
    int ret = A_NONE;
    int shift = 0;

    if (keycode >= TK_MOUSE_LEFT && keycode <= TK_MOUSE_CLICKS) {
        ret = handle_mouse(keycode);
    }
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
    } else if (keycode == TK_O) {
        ret = A_OPEN;
    } else if (keycode == TK_C) {
        ret = A_CLOSE;
    } else if (keycode == TK_SEMICOLON && !shift) {
        return A_LOOK;
    } else if ((keycode == TK_PERIOD) && shift) {
        ret = A_DESCEND;
    } else if ((keycode == TK_COMMA) && shift) {
        ret = A_ASCEND;
    } else if (keycode == TK_COMMA) {
        ret = A_PICK_UP;
    } else if (keycode == TK_PERIOD) {
        ret = A_REST;
    } else if (keycode == TK_P) {
        ret = A_FULLSCREEN;
    } else if (keycode == TK_X) {
        ret = A_EXPLORE;
    } else if (keycode == TK_I) {
        ret = A_INVENT;
    } else if ((keycode == TK_SLASH) && shift) {
        ret = A_HELP;
    } else if ((keycode == TK_SEMICOLON) && shift) {
        ret = A_LOOK_DOWN;
    } else if ((keycode == TK_S) && shift) {
        ret = A_SAVE;
    } else if (((keycode == TK_Q) && shift) || keycode == TK_ESCAPE) {
        ret = A_QUIT;
    } else if ((keycode == TK_R) && terminal_check(TK_CONTROL)) {
        ret = A_MAGICMAP;
    } else if ((keycode == TK_E) && terminal_check(TK_CONTROL)) {
        ret = A_HEAT;
    }
    /* Toggle runmode */
    if (is_movement(ret) && shift) {
        f.mode_run = f.mode_map;
    }
    return ret;
}

struct actor *win_pick_invent(void) {
    return 0;
}