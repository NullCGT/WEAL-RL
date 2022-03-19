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

/* SCREEN FUNCTIONS */

/* Perform the first-time setup for the game's GUI. */
void setup_gui(void) {
    g.map_win = create_win(MAPWIN_H, MAPWIN_W, MAPWIN_Y, 0);
    g.msg_win = create_win(MSG_H, MSG_W, MSG_Y, 0);
    draw_msg_window(g.msg_win);
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
    initscr();
    if (has_colors()) {
        start_color();
        setup_colors();
    }
    setup_locale();
    noecho();
    raw();
    keypad(stdscr, TRUE);
    mousemask(ALL_MOUSE_EVENTS, NULL);
    putenv("ESCDELAY=25");
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
        sprintf(buf, "%c %d/%d", cur_npc->chr, cur_npc->energy, cur_npc->emax);
        mvwprintw(new_win, 1, 1, buf);
        render_bar(new_win, cur_npc->energy, cur_npc->emax, 1, 2, SB_W - 2, ACS_CKBOARD, '_');
        cur_npc = cur_npc->next;
    }
    wrefresh(new_win);
}