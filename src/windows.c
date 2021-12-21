#include <curses.h>
/* #include <menu.h> */
#include <stdlib.h>
#include <locale.h>
#include <string.h>

#include "register.h"
#include "windows.h"
#include "message.h"

void setup_gui(void);
void setup_locale(void);
void setup_colors(void);

/* SCREEN FUNCTIONS */

/* Perform the first-time setup for the game's GUI. */
void setup_gui(void) {
    g.map_win = create_win(MAP_HEIGHT, MAP_WIDTH, 1, 1);
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
    setlocale(LC_ALL, "en-us");
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
    int w = MAP_WIDTH / 2;
    int h = MAP_HEIGHT / 2;
    
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
    return;
}

/* OUTPUT FUNCTIONS */

/* Outputs a character to the map window. Wrapper for mvwaddch(). */
int map_putch(int y, int x, int chr) {
    return mvwaddch(g.map_win, y, x, chr); 
}

#if 0
int create_menu(struct action *actions, char *title)
{	ITEM **my_items;
	int c;				
	MENU *my_menu;
    WINDOW *my_menu_win;
    int n_choices, i;
    struct action *cur_action = actions;


    /* ITEM CREATION */
    while (cur_action != NULL) {
        n_choices++;
        cur_action = cur_action->next;
    }
    i = 0;
    my_items = (ITEM **)calloc(n_choices, sizeof(ITEM *));
    cur_action = actions;
	while (cur_action != NULL) {
        my_items[i] = new_item(cur_action->name, cur_action->desc);
        cur_action = cur_action->next;
        i++;
    }

	/* Create menu */
	my_menu = new_menu((ITEM **)my_items);

	/* Set menu option not to show the description */
	menu_opts_off(my_menu, O_SHOWDESC);

	/* Create the window to be associated with the menu */
    my_menu_win = newwin(10, 70, 4, 4);
    keypad(my_menu_win, TRUE);
     
	/* Set main window and sub window */
    set_menu_win(my_menu, my_menu_win);
    set_menu_sub(my_menu, derwin(my_menu_win, 6, 68, 3, 1));
	set_menu_format(my_menu, 5, 3);
	set_menu_mark(my_menu, " > ");

	/* Print a border around the main window and print a title */
    box(my_menu_win, 0, 0);

    /* Set the title */
    mvwprintw(my_menu_win, 0, 1, title);

	/* Post the menu */
	post_menu(my_menu);
	wrefresh(my_menu_win);
	
	while((c = wgetch(my_menu_win)) != 'x')
	{       
        switch(c)
        {
        case KEY_DOWN:
            menu_driver(my_menu, REQ_DOWN_ITEM);
            break;
        case KEY_UP:
            menu_driver(my_menu, REQ_UP_ITEM);
            break;
        case KEY_LEFT:
            menu_driver(my_menu, REQ_LEFT_ITEM);
            break;
        case KEY_RIGHT:
            menu_driver(my_menu, REQ_RIGHT_ITEM);
            break;
        case KEY_NPAGE:
            menu_driver(my_menu, REQ_SCR_DPAGE);
            break;
        case KEY_PPAGE:
            menu_driver(my_menu, REQ_SCR_UPAGE);
            break;
        case KEY_HOME:
            menu_driver(my_menu, REQ_FIRST_ITEM);
            break;
        case KEY_END:
            menu_driver(my_menu, REQ_LAST_ITEM);
            break;
        }
        wrefresh(my_menu_win);
	}

	/* Unpost and free all the memory taken up */
    unpost_menu(my_menu);
    free_menu(my_menu);
    for(i = 0; i < n_choices; ++i) {
        free_item(my_items[i]);
    }
	cleanup_win(my_menu_win);
    f.update_map = 1;
    return 0;
}
#endif