#include <notcurses/notcurses.h>
#include <stdlib.h>
#include <locale.h>
#include <string.h>
#include <unistd.h>

#include "register.h"
#include "windows.h"
#include "render.h"
#include "message.h"
#include "action.h"

void setup_gui(void);
void setup_locale(void);
void setup_colors(void);
int handle_mouse(void);

#define MAX_FILE_LEN 300

/* Variables */
struct notcurses *nc;
struct ncplane* nstd;
struct ncplane* nmsg_plane;

static unsigned colors[] = {
    0x000000, // Black
    0xFF0000, // Red
    0x008000, // Green
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
        .y = 0,
        .x = 0,
        .rows = MSG_H,
        .cols = MSG_W,
        .userptr = NULL,
        .name = "Log Plane",
        .resizecb = NULL,
        .flags = 0
    };
    nmsg_plane =  ncplane_create(nstd, &msgplane_opts);
    ncplane_set_scrolling(nmsg_plane, 1);
}

/* Set the locale of the terminal for the purposes of consistency, bug
   reproducibility, and drawing special characters. The previous locale
   is saved and reset upon game exit. */
void setup_locale(void) {
    char *old_locale;
    old_locale = setlocale(LC_ALL, NULL);
    g.saved_locale = strdup(old_locale);
    if (g.saved_locale == NULL)
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

    nstd = notcurses_stddim_yx(nc, &h, &w);
    if (h > 1 && w > 1) {
        term.h = h;
        term.w = w;
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
    /* Set up new plane */
    struct ncplane_options text_plane_opts = {
        .y = 0,
        .x = 0,
        .rows = MAX_FILE_LEN,
        .cols = MSG_W + MAPWIN_W,
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

/* Outputs a character to the map window. Wrapper for mvwaddch(). */
int map_putch(int x, int y, int chr, int attr) {
    int ret;
    ncplane_set_fg_rgb(nstd, colors[attr]);
    ret = ncplane_putchar_yx(nstd, y + MAPWIN_Y, x, (wchar_t) chr);
    ncplane_set_fg_rgb(nstd, colors[WHITE]);
    return ret;
}

int map_putch_truecolor(int x, int y, int chr, unsigned color) {
    int ret;
    ncplane_set_fg_rgb(nstd, color);
    ret = ncplane_putchar_yx(nstd, y + MAPWIN_Y, x, (wchar_t) chr);
    ncplane_set_fg_rgb(nstd, colors[WHITE]);
    return ret;
}

void clear_map(void) {
    ncplane_erase(nstd);
}

void refresh_map(void) {
    notcurses_render(nc);
}

/* Handle key inputs. Blocking. */
int handle_keys(void) {
    struct ncinput input;
    notcurses_get(nc, NULL, &input);
    if (input.evtype == NCTYPE_RELEASE)
        return A_NONE;
    switch(input.id) {
        case 'h':
        case NCKEY_LEFT:
        case '4':
            return A_WEST;
        case 'j':
        case NCKEY_DOWN:
        case '2':
            return A_SOUTH;
        case 'k':
        case NCKEY_UP:
        case '8':
            return A_NORTH;
        case 'l':
        case NCKEY_RIGHT:
        case '6':
            return A_EAST;
        case 'y':
        case NCKEY_ULEFT:
        case '7':
            return A_NORTHWEST;
        case 'u':
        case NCKEY_URIGHT:
        case '9':
            return A_NORTHEAST;
        case 'n':
        case NCKEY_DRIGHT:
        case '3':
            return A_SOUTHEAST;
        case 'b':
        case NCKEY_DLEFT:
        case '1':
            return A_SOUTHWEST;
        case '.':
            if (input.modifiers & NCKEY_MOD_SHIFT)
                return A_DESCEND;
            /* FALLTHRU */
        case NCKEY_CENTER:
        case '5':
            return A_REST;
        case ',':
            if (input.modifiers & NCKEY_MOD_SHIFT)
                return A_ASCEND;
            break;
        case 'p':
            return A_FULLSCREEN;
        case 'x':
            return A_EXPLORE;
        case 'Q':
        case NCKEY_EXIT:
        case NCKEY_ESC:
            return A_QUIT;
        case '/':
            if (input.modifiers & NCKEY_MOD_SHIFT)
                return A_HELP;
            break;
        case 'R':
            if (input.modifiers & NCKEY_MOD_CTRL)
                return A_DEBUG_MAGICMAP;
            break;
        case 'E':
            if (input.modifiers & NCKEY_MOD_CTRL)
                return A_DEBUG_HEAT;
            break;
        default:
            break;
    }
    return A_NONE;
}