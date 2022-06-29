#include <notcurses/notcurses.h>
#include <stdlib.h>
#include <locale.h>
#include <string.h>

#include "register.h"
#include "windows.h"
#include "render.h"
#include "message.h"
#include "action.h"
#include "map.h"
#include "invent.h"

void setup_gui(void);
void refresh_gui(void);
void setup_locale(void);
void setup_colors(void);
int handle_mouse(struct ncinput);

#define MAX_FILE_LEN 300

/* Variables */
struct notcurses *nc;
struct ncplane* nstd;
struct ncplane* nmsg_plane;
struct ncplane* nmsgui_plane;

static unsigned colors[] = {
    0x000000, // Black
    0xFF0000, // Red
    0x00FF00, // Green
    0xFFFF00, // Yellow
    0x0000FF, // Blue
    0xFF00FF, // Magenta
    0x00FFFF, // Cyan
    0xFFFFFF  // White
};

/* SCREEN FUNCTIONS */

/* Perform the first-time setup for the game's GUI. */
void setup_gui(void) {
    struct ncplane_options msgplane_opts = {
        .y = 1,
        .x = 1,
        .rows = term.msg_h - 2,
        .cols = term.msg_w - 2,
        .userptr = NULL,
        .name = "Log Plane",
        .resizecb = NULL,
        .flags = 0
    };
    struct ncplane_options msguiplane_opts = {
        .y = 0,
        .x = 0,
        .rows = term.msg_h,
        .cols = term.msg_w,
        .userptr = NULL,
        .name = "Log Plane",
        .resizecb = NULL,
        .flags = 0
    };
    nmsgui_plane = ncplane_create(nstd, &msguiplane_opts);
    refresh_gui();
    nmsg_plane =  ncplane_create(nmsgui_plane, &msgplane_opts);
    ncplane_set_scrolling(nmsg_plane, 1);
}

/* Refresh the message plane GUI */
void refresh_gui(void) {
    ncplane_double_box(nmsgui_plane, 0, 0, term.msg_h - 1, term.msg_w - 1, 0);
    ncplane_printf_aligned(nmsgui_plane, 0, NCALIGN_CENTER, "Messages");
}

/* Set the locale of the terminal for the purposes of consistency, bug
   reproducibility, and drawing special characters. The previous locale
   is saved and reset upon game exit. */
void setup_locale(void) {
    char *old_locale;
    old_locale = setlocale(LC_ALL, NULL);
    term.saved_locale = strdup(old_locale);
    if (term.saved_locale == NULL)
        return;
    setlocale(LC_ALL, "en_US.UTF-8");
    return;
}

/* Set up the the scren of the game. In addition to creating the main window,
   this initializes the screen, turns off echoing, and does the basic setup
   needed for curses to do its job. */
void setup_screen(void) {
    unsigned int h, w;
    
    putenv("ESCDELAY=25");
    setup_locale();
    
    notcurses_options nopts;
    memset(&nopts, 0, sizeof(nopts));
    if ((nc = notcurses_init(&nopts, stdout)) == NULL) {
        exit(1);
    }
    notcurses_mice_enable(nc, NCMICE_MOVE_EVENT | NCMICE_BUTTON_EVENT);

    nstd = notcurses_stddim_yx(nc, &h, &w);
    if (h > 1 && w > 1) {
        setup_term_dimensions(h, w, 1, 1);
    } else if ((h > 1 && h < MIN_TERM_H) || (w > 1 && w < MIN_TERM_W)) {
        // printf("Terminal must be at least %dx%d.", MIN_TERM_W, MIN_TERM_H);
        notcurses_stop(nc);
        exit(1);
    }
    setup_gui();
    notcurses_render(nc);
}

/* The counterpart to setup_screen(). Called at the end of the program, and
   used to clean up curses artifacts. */
void cleanup_screen(void) {
    notcurses_stop(nc);
    return;
}

void display_file_text(const char *fname) {
    FILE *fp;
    struct ncplane *text_plane;
    char *line = NULL;
    size_t len = 0;
    int action;

    /* Open the file for reading */
    fp = fopen(fname, "r");
    if (fp == NULL)
        return;
    /* Clear existing planes */
    ncplane_erase(nmsg_plane);
    ncplane_erase(nstd);
    ncplane_erase(nmsgui_plane);
    f.mode_map = 0;
    /* Set up new plane */
    struct ncplane_options text_plane_opts = {
        .y = 0,
        .x = 0,
        .rows = MAX_FILE_LEN,
        .cols = term.w,
        .userptr = NULL,
        .name = "File Text Plane",
        .resizecb = NULL,
        .flags = 0
    };
    text_plane =  ncplane_create(nstd, &text_plane_opts);
    ncplane_set_scrolling(text_plane, 1);
    /* Write file to new plane */
    while (getline(&line, &len, fp) != -1) {
        ncplane_printf(text_plane, "%s", line);
    }
    free(line);
    fclose(fp);
    /* Handle player input */
    while (1) {
        notcurses_render(nc);
        action = handle_keys();
        switch (action) {
            case A_NORTH:
            case A_ASCEND:
                ncplane_move_rel(text_plane, 1, 0);
                break;
            case A_SOUTH:
            case A_DESCEND:
                ncplane_move_rel(text_plane, -1, 0);
                break;
            case A_QUIT:
            case A_HELP:
                ncplane_destroy(text_plane);
                f.update_map = 1;
                f.update_msg = 1;
                f.mode_map = 1;
                refresh_gui();
                return;
        }
    }
}

void create_popup_win(const char *title, const char *msg) {
    (void) title;
    (void) msg;
}

void display_energy_win(void) {
}

void draw_msg_window(int h, int full) {
    struct msg *cur_msg = g.msg_last;
    ncplane_erase(nmsg_plane);
    (void) h;
    (void) full;
    /* TODO: Grab cursor position in order to prematurely exit
       the loop and save some printing and rendering time. */
    while (cur_msg != NULL) {
        ncplane_set_fg_rgb(nmsg_plane, colors[cur_msg->attr]);
        ncplane_printf(nmsg_plane, "%s\n", cur_msg->msg);
        cur_msg = cur_msg->prev;
    }
    f.update_msg = 0;
}

int map_put_tile(int x, int y, int mx, int my, int color) {
    int ret;
    ncplane_set_fg_rgb(nstd, colors[color]);
    ret = ncplane_putwc_yx(nstd, y + term.mapwin_y, x, g.levmap[mx][my].pt->wchr);
    ncplane_set_fg_rgb(nstd, colors[WHITE]);
    return ret;
}

int map_put_actor(int x, int y, struct actor *actor, int attr) {
    return map_putch(x, y, actor->chr, attr);
}

int map_putch(int x, int y, int chr, int attr) {
    int ret;
    ncplane_set_fg_rgb(nstd, colors[attr]);
    ret = ncplane_putchar_yx(nstd, y + term.mapwin_y, x, (wchar_t) chr);
    ncplane_set_fg_rgb(nstd, colors[WHITE]);
    return ret;
}

int map_putch_truecolor(int x, int y, int chr, unsigned color) {
    int ret;
    ncplane_set_fg_rgb(nstd, color);
    ret = ncplane_putchar_yx(nstd, y + term.mapwin_y, x, (wchar_t) chr);
    ncplane_set_fg_rgb(nstd, colors[WHITE]);
    return ret;
}

void clear_map(void) {
    ncplane_erase(nstd);
}

void refresh_map(void) {
    notcurses_render(nc);
}

int handle_mouse(struct ncinput input) {
    int gx = input.x + g.cx;
    int gy = input.y - g.cy - term.mapwin_y;
    if (f.mode_look) {
        g.cursor_x = gx;
        g.cursor_y = gy;
    }
    if (input.evtype == NCTYPE_RELEASE && input.id == NCKEY_BUTTON1
        && in_bounds(gx, gy) && is_explored(gx, gy) && !is_blocked(gx, gy)) {
        g.goal_x = gx;
        g.goal_y = gy;
        f.mode_run = 1;
        return A_NONE;
    }
    if (input.evtype == NCTYPE_PRESS && input.id == NCKEY_BUTTON3) {
        look_at(input.x + g.cx, input.y - g.cy - term.mapwin_y);
    }
    return A_NONE;
}

/* Handle key inputs. Blocking. */
int handle_keys(void) {
    struct ncinput input;
    int shift;
    int ret = A_NONE;

    notcurses_get(nc, NULL, &input);
    if (input.x && input.y)
        handle_mouse(input);
    if (input.evtype == NCTYPE_RELEASE)
        return ret;
    shift = (input.modifiers & NCKEY_MOD_SHIFT);
    switch(input.id) {
        case 'h':
        case 'H':
        case NCKEY_LEFT:
        case '4':
            ret = A_WEST;
            break;
        case 'j':
        case 'J':
        case NCKEY_DOWN:
        case '2':
            ret = A_SOUTH;
            break;
        case 'k':
        case 'K':
        case NCKEY_UP:
        case '8':
            ret = A_NORTH;
            break;
        case 'l':
        case 'L':
        case NCKEY_RIGHT:
        case '6':
            ret = A_EAST;
            break;
        case 'y':
        case 'Y':
        case NCKEY_ULEFT:
        case '7':
            ret = A_NORTHWEST;
            break;
        case 'u':
        case 'U':
        case NCKEY_URIGHT:
        case '9':
            ret = A_NORTHEAST;
            break;
        case 'n':
        case 'N':
        case NCKEY_DRIGHT:
        case '3':
            ret = A_SOUTHEAST;
            break;
        case 'b':
        case 'B':
        case NCKEY_DLEFT:
        case '1':
            ret = A_SOUTHWEST;
            break;
        case '.':
            if (shift) {
                ret = A_DESCEND;
                break;
            }
            /* FALLTHRU */
        case NCKEY_CENTER:
        case '5':
            ret = A_REST;
            break;
        case 'o':
            ret = A_OPEN;
            break;
        case 'c':
            ret = A_CLOSE;
            break;
        case ',':
            if (shift) {
                ret = A_ASCEND;
            } else {
                ret = A_PICK_UP;
            }
            break;
        case 'p':
            ret = A_FULLSCREEN;
            break;
        case 'x':
            ret = A_EXPLORE;
            break;
        case 'i':
            ret = A_INVENT;
            break;
        case 'S':
            ret = A_SAVE;
            break;
        case 'Q':
        case NCKEY_EXIT:
        case NCKEY_ESC:
            ret = A_QUIT;
            break;
        case '/':
            if (shift)
                ret = A_HELP;
            break;
        case ';':
            if (shift) {
                ret = A_LOOK_DOWN;
                break;
            } else {
                return A_LOOK;
            }
            break;
        case 'R':
            if (input.modifiers & NCKEY_MOD_CTRL)
                ret = A_MAGICMAP;
            break;
        case 'E':
            if (input.modifiers & NCKEY_MOD_CTRL)
                ret = A_HEAT;
            break;
        default:
            break;
    }
    /* Toggle runmode */
    if (shift && is_movement(ret))
        f.mode_run = f.mode_map;
    return ret;
}

void menu_destroy(struct ncselector *selector) {
    ncselector_destroy(selector, 0);
}

struct ncselector *menu_new(int x, int y, int rows, int cols) {
    struct ncplane *selector_plane;
    struct ncselector *selector;
    struct ncplane_options plane_opts = {
        .y = x,
        .x = y,
        .rows = rows,
        .cols = cols,
        .userptr = NULL,
        .name = "Menu Plane",
        .resizecb = NULL,
        .flags = 0
    };
    struct ncselector_options select_options = {
        .title = NULL,
        .secondary = NULL,
        .footer = NULL,
        .items = NULL,
        .defidx = 0,
        .maxdisplay = min(term.h, 26),
        .opchannels = 0,
        .descchannels = 0,
        .titlechannels = 0,
        .footchannels = 0,
        .boxchannels = 0,
        .flags = 0
    };
    selector_plane = ncplane_create(nstd, &plane_opts);
    selector = ncselector_create(selector_plane, &select_options);
    f.mode_map = 0;

    return selector;
}

void menu_add_item(struct ncselector *selector, const char *choice, const char *text) {
    #if 0
    struct ncselector_item *new_item = malloc(sizeof(struct ncselector_item));
    new_item->option = choice;
    new_item->desc = text;
    #endif
    struct ncselector_item new_item = { choice, text };
    ncselector_additem(selector, &new_item);
}

/* Returns char selected from menu, or -1 if an invalid selection was made. */
unsigned int menu_do_choice(struct ncselector *selector, int can_quit) {
    struct ncinput input;

    while (1) {
        notcurses_get(nc, NULL, &input);
        ncselector_offer_input(selector, &input);
        if (input.id == NCKEY_ESC && can_quit) {
            return -1;
        } else if (input.id == NCKEY_ENTER && input.evtype != NCTYPE_RELEASE) {
            return ncselector_selected(selector)[0];
        } else if (input.id >= 'a' && input.id <= 'z' && input.evtype != NCTYPE_RELEASE) {
            return input.id;
        }
    }
}

struct actor *win_pick_invent(void) {
    struct ncselector *selector;
    struct actor *cur = g.player->invent;
    int selected = -1;

    if (cur == NULL) {
        logm("I am not carrying anything.");
        return NULL;
    }

    selector = menu_new(1, 1, term.w - 2, term.h - 2);
    while (cur != NULL) {
        /* To resolve valgrind issues here: Allocate memory for each of these.
           Notcurses frees the memory of item members when a window is destroyed. */
        //menu_add_item(selector, &cur->item->letter, actor_name(cur, 0));
        menu_add_item(selector, "a", "fart");
        cur = cur->next;
    }
    notcurses_render(nc);
    while (1) {
        selected = menu_do_choice(selector, 1);
        cur = g.player->invent;
        if (selected == -1) {
            cur = NULL;
            break;
        }
        while (cur != NULL) {
            if (cur->item->letter == selected) {
                menu_destroy(selector);
                f.mode_map = 1;
                return cur;
            }
            cur = cur->next;
        }
    }
    menu_destroy(selector);
    f.mode_map = 1;
    return cur;
}