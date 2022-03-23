#include <curses.h>
#include <stdlib.h>
#include <locale.h>
#include <string.h>

#include "register.h"
#include "windows.h"
#include "render.h"
#include "message.h"

void setup_gui(void);
void setup_locale(void);
void setup_colors(void);
void render_bar(WINDOW*, int, int, int, int, int, int, int);
void draw_msg_window(WINDOW *, int);

/* SCREEN FUNCTIONS */

/* Perform the first-time setup for the game's GUI. */
void setup_gui(void) {
    g.map_win = create_win(MAPWIN_H, MAPWIN_W, MAPWIN_Y, 0);
    g.msg_win = create_win(MSG_H, MSG_W, MSG_Y, 0);
    f.update_map = 1;
    draw_msg_window(g.msg_win, MSG_H);
    wrefresh(g.map_win);
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

void display_energy_win(void) {
    WINDOW* new_win;
    char buf[128];
    struct npc *cur_npc = &g.player;

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

        mvwprintw(new_win, 5, 1, "Energy");
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

/* TODO: Use waddstr in and scrollok in order to take a lot of the complexity out of this and
   also support screen wrapping. Note: May require rewriting the linked list of messages to
   allow backwards traversal. */
/* TODO: Pdcurses has issues with getmaxyx, so we will need to include our own version of the
   header. That will allow us to cut down on the arguments. */
void draw_msg_window(WINDOW *win, int h) {
    int i = 0;
    int x, y;
    struct msg *cur_msg;
    struct msg *prev_msg;

    werase(win);
    cur_msg = g.msg_list;
    while (cur_msg != NULL) {
        getyx(win, y, x);
        (void) x;
        if (i >= MAX_BACKSCROLL) {
            prev_msg->next = NULL;
            prev_msg = cur_msg;
            cur_msg = cur_msg->next;
            free_msg(prev_msg);
            i++;
            continue;
        } else if (y > h - 2) {
            prev_msg = cur_msg;
            cur_msg = cur_msg->next;
            i++;
            continue;
        }
        wattron(win, cur_msg->attr);
        waddstr(win, cur_msg->msg);
        wattroff(win, cur_msg->attr);
        waddch(win, '\n');
        prev_msg = cur_msg;
        cur_msg = cur_msg->next;
        i++;
    }
    wrefresh(win);
    f.update_msg = 0;
}

void full_msg_window(void) {
    WINDOW *win;
    win = create_win(MAPWIN_H + MSG_H, MSG_W, 0, 0);
    draw_msg_window(win, term.h);
    wrefresh(win);
    getch();
    cleanup_win(win);
    f.update_map = 1;
    f.update_msg = 1;
    return;
}

/* Outputs a character to the map window. Wrapper for mvwaddch(). */
int map_putch(int y, int x, int chr) {
    return mvwaddch(g.map_win, y, x, chr); 
}

void refresh_map(void) {
    wmove(g.map_win, g.player.y - g.cy, g.player.x - g.cx);
    wrefresh(g.map_win);
}