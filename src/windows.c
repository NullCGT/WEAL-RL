#include <curses.h>
#include <stdbool.h>
#include <locale.h>

#include "windows.h"
#include "register.h"

extern struct global g;

/* SCREEN FUNCTIONS */

/* Set up the the scren of the game. In addition to creating the main window,
   this initializes the screen, turns off echoing, and does the basic setup
   needed for curses to do its job. */
WINDOW* setup_screen(void) {
    WINDOW * main_win;
    initscr();
    noecho();
    raw();
    keypad(stdscr, TRUE);
    refresh();
    main_win = create_win(MAP_HEIGHT + 2, MAP_WIDTH + 2, 0, 0);
    box(main_win, 0, 0);
    wrefresh(main_win);
    return main_win;
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

/* Clean up a window by refreshing it, then deleting it. */
void cleanup_win(WINDOW *win) {
    wrefresh(win);
    delwin(win);
    return;
}

/* OUTPUT FUNCTIONS */

/* Outputs a character to the map window. Wrapper for mvwaddch(). */
int map_putch(int y, int x, int chr) {
    return mvwaddch(g.map_win, y, x, chr); 
}