#include <curses.h>
#include <stdlib.h>
#include <stdio.h>
#include <locale.h>
#include <string.h>

#include "register.h"
#include "windows.h"
#include "render.h"
#include "message.h"
#include "action.h"

void setup_gui(void);
void setup_locale(void);
void setup_colors(void);
WINDOW* create_win(int, int, int, int);
void cleanup_win(WINDOW *);
void render_bar(WINDOW*, int, int, int, int, int, int, int);
int handle_mouse(void);

#define MAX_FILE_LEN 200

WINDOW *map_win;
WINDOW *msg_win;

/* SCREEN FUNCTIONS */

/* Perform the first-time setup for the game's GUI. */
void setup_gui(void) {
    map_win = create_win(MAPWIN_H, MAPWIN_W, MAPWIN_Y, 0);
    msg_win = create_win(MSG_H, MSG_W, MSG_Y, 0);
    f.update_map = 1;
    draw_msg_window(MSG_H, 0);
    wrefresh(map_win);
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
    int h, w;
    putenv("ESCDELAY=25");
    initscr();
    getmaxyx(stdscr, h, w);
    if (h > 1 && w > 1) {
        term.h = h;
        term.w = w;
    } else if ((h > 1 && h < MIN_TERM_H) || (w > 1 && w < MIN_TERM_W)) {
        printf("Terminal must be at least %dx%d.", MIN_TERM_W, MIN_TERM_H);
        exit(0);
    }
    if (has_colors()) {
        start_color();
        setup_colors();
    }
    setup_locale();
    noecho();
    raw();
    keypad(stdscr, TRUE);
    mousemask(ALL_MOUSE_EVENTS, NULL);
    refresh();
    setup_gui();
}

/* Set up the color pairs necessary for rendering the game in color. This function
   is only called if color is supported by the terminal. */
void setup_colors(void) {
    for (int i = COLOR_BLACK; i <= COLOR_WHITE; i++) {
        init_pair(i, i, COLOR_BLACK);
    }
    return;
}

/* The counterpart to setup_screen(). Called at the end of the program, and
   used to clean up curses artifacts. */
void cleanup_screen(void) {
    endwin();
    return;
}

/* WINDOW MANAGEMENT FUNCTIONS */

/* Create a new window. Wrapper for curses function newwin. */
WINDOW* create_win(int h, int w, int y, int x) {
    WINDOW* new_win;
    new_win = newwin(h, w, y, x);
    return new_win;
}

/* Clean up a window by erasing it, then deleting it. */
void cleanup_win(WINDOW *win) {
    werase(win);
    wrefresh(win);
    delwin(win);
    return;
}

void create_popup_win(const char *title, const char *msg) {
    WINDOW* new_win;
    int w = MAPWIN_W / 2;
    int h = MAPWIN_H / 2;
    
    new_win = newwin(h, w, h /2, w / 2);
    box(new_win, 0, 0);
    wattron(new_win, COLOR_PAIR(GREEN));
    mvwprintw(new_win, 0, 1, title);
    mvwprintw(new_win, h - 1, 1, "[Press back to dismiss.]");
    wattroff(new_win, COLOR_PAIR(GREEN));
    mvwprintw(new_win, 1, 1, msg);
    wrefresh(new_win);
    while (getch() != 'x');
    cleanup_win(new_win);
    f.update_map = 1;
    f.update_msg = 1;
    return;
}

void display_file_text(const char *fname) {
    FILE *fp;
    WINDOW *new_win;
    int i =0;
    int j = 0;
    int action = A_NONE;
    char *line = NULL;
    size_t len = 0;

    new_win = newpad(MAX_FILE_LEN, term.w);
    /* Write the file to the window */
    fp = fopen(fname, "r");
    if (fp == NULL)
        return;
    while (getline(&line, &len, fp) != -1) {
        mvwprintw(new_win, i++, 0, line);
    }
    fclose(fp);
    /* Handle player input */
    while (1) {
        prefresh(new_win, j, 0, 0, 0, term.h - 1, term.w - 1);

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
                cleanup_win(new_win);
                f.update_map = 1;
                f.update_msg = 1;
                return;
        }
        j = max(0, j);
    }
}

void display_energy_win(void) {
    WINDOW* new_win;
    char buf[128];
    struct actor *cur_npc = &g.player;

    new_win = newwin(SB_H, SB_W, SB_Y, SB_X);
    box(new_win, 0, 0);
    
    while(cur_npc != NULL) {
        sprintf(buf, "Player Location: (%d, %d)", cur_npc->x, cur_npc->y);
        mvwprintw(new_win, 1, 1, buf);
        memset(buf, 0, 128);
        sprintf(buf, "Camera Origin: (%d, %d)", g.cx, g.cy);
        mvwprintw(new_win, 2, 1, buf);
        memset(buf, 0, 128);
        sprintf(buf, "Turn: %d", g.turns);
        mvwprintw(new_win, 3, 1, buf);
        memset(buf, 0, 128);
        sprintf(buf, "Depth: %d meters", g.depth * 4);
        mvwprintw(new_win, 4, 1, buf);

        mvwprintw(new_win, 6, 1, "Energy");
        render_bar(new_win, cur_npc->energy, cur_npc->emax, 1, 6, SB_W - 2, ACS_CKBOARD, '_');
        
        cur_npc = cur_npc->next;
    }
    wrefresh(new_win);
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

void draw_msg_window(int h, int full) {
    int i = 0;
    int x, y;
    struct msg *cur_msg;
    WINDOW *win;

    if (full) {
        win = create_win(MAPWIN_H + MSG_H, MSG_W, 0, 0);
    } else {
        win = msg_win;
    }

    werase(win);
    cur_msg = g.msg_list;
    while (cur_msg != NULL) {
        getyx(win, y, x);
        (void) x;
        if (y > h - 2) {
            cur_msg = cur_msg->next;
            i++;
            continue;
        }
        wattron(win, COLOR_PAIR(cur_msg->attr));
        waddstr(win, cur_msg->msg);
        wattroff(win, COLOR_PAIR(cur_msg->attr));
        waddch(win, '\n');
        cur_msg = cur_msg->next;
        i++;
    }
    wrefresh(win);
    f.update_msg = 0;

    if (full) {
        getch();
        cleanup_win(win);
        f.update_map = 1;
        f.update_msg = 1;
    }
}

/* Outputs a character to the map window. Wrapper for mvwaddch(). */
int map_putch(int x, int y, int chr, int attr) {
    int ret;
    wattron(map_win, COLOR_PAIR(attr));
    ret = mvwaddch(map_win, y, x, chr); 
    wattroff(map_win, COLOR_PAIR(attr));
    return ret;
}

/* Outputs a character to the map window. Since curses only supports
   a small set of colors, we cast to an integer and modulo. */
int map_putch_truecolor(int x, int y, int chr, unsigned color) {
    int final_color = color % COLOR_MAX;
    return map_putch(x, y, chr, final_color);
}

void clear_map(void) {
    werase(map_win);
}

void refresh_map(void) {
    wmove(map_win, g.player.y - g.cy, g.player.x - g.cx);
    wrefresh(map_win);
}

/* handle mouse inputs */
int handle_mouse(void) {
    int x, y;
    MEVENT event;

    if (getmouse(&event) != OK)
        return A_NONE;
    
    x = event.x;
    y = event.y;
    
    if (event.bstate & BUTTON1_CLICKED) {
        if (y <= MSG_H - 1 && x <= MSG_W) {
            return A_FULLSCREEN;
        }
    }
    return A_NONE;
}

/* Handle key inputs. Blocking. */
int handle_keys(void) {
    int keycode = getch();
    /* This is a bit more complicated than other input systems,
       since curses picks up character codes, rather than
       keyboard strikes. */
    switch(keycode) {
        case 'H':
            f.mode_run = 1;
            /* FALLTHRU */
        case 'h':
        case KEY_LEFT:
        case '4':
            return A_WEST;
        case 'J':
            f.mode_run = 1;
            /* FALLTHRU */
        case 'j':
        case KEY_DOWN:
        case '2':
            return A_SOUTH;
        case 'K':
            f.mode_run = 1;
            /* FALLTHRU */
        case 'k':
        case KEY_UP:
        case '8':
            return A_NORTH;
        case 'L':
            f.mode_run = 1;
            /* FALLTHRU */
        case 'l':
        case KEY_RIGHT:
        case '6':
            return A_EAST;
        case 'Y':
            f.mode_run = 1;
            /* FALLTHRU */
        case 'y':
        case '7':
            return A_NORTHWEST;
        case 'U':
            f.mode_run = 1;
            /* FALLTHRU */
        case 'u':
        case '9':
            return A_NORTHEAST;
        case 'N':
            f.mode_run = 1;
            /* FALLTHRU */
        case 'n':
        case '3':
            return A_SOUTHEAST;
        case 'B':
            f.mode_run = 1;
            /* FALLTHRU */
        case 'b':
        case '1':
            return A_SOUTHWEST;
        case '.':
        case '5':
            return A_REST;
        case KEY_MOUSE:
            return handle_mouse();
        case 'p':
            return A_FULLSCREEN;
        case '>':
            return A_DESCEND;
        case '<':
            return A_ASCEND;
        case 'x':
            return A_EXPLORE;
        case '?':
            return A_HELP;
        case 'Q':
            return A_QUIT;
        default:
            break;
    }
    /* Handle combination keys */
    const char *key_name = keyname(keycode);
    if (!strncmp(key_name, "^R", 2))
        return A_DEBUG_MAGICMAP;
    if (!strncmp(key_name, "^E", 2))
        return A_DEBUG_HEAT;
    return A_NONE;
}