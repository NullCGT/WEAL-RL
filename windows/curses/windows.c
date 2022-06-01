/**
 * @file windows.c
 * @author Kestrel (kestrelg@kestrelscry.com)
 * @brief SCreen and window-related functions for the ncurses window port.
 * @version 1.0
 * @date 2022-05-27
 * 
 * @copyright Copyright (c) 2022
 * 
 */

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
#include "map.h"
#include "invent.h"
#include "menu.h"

void setup_gui(void);
void setup_locale(void);
void setup_colors(void);
void render_bar(WINDOW*, int, int, int, int, int, int, int);
int handle_mouse(void);

#define MAX_FILE_LEN 200

struct curse_color {
    int r, g, b;
};

#define CHANGE_COLORS 0

WINDOW *map_win;
WINDOW *msg_win;

/* SCREEN FUNCTIONS */

/**
 * @brief Perform the first-time setup for the game's GUI.
 * 
 */
void setup_gui(void) {
    map_win = create_win(term.mapwin_h, term.mapwin_w, term.mapwin_y, 0);
    msg_win = create_win(term.msg_h, term.msg_w, 0, 0);
    f.update_map = 1;
    draw_msg_window(term.msg_h, 0);
    wrefresh(map_win);
}

/**
 * @brief Set the locale of the terminal for the purposes of consistency, bug
reproducibility, and drawing special characters. The previous locale
is saved and reset upon game exit.
 * 
 */
void setup_locale(void) {
    char *old_locale;
    old_locale = setlocale(LC_ALL, NULL);
    term.saved_locale = strdup(old_locale);
    if (term.saved_locale == NULL)
        return;
    setlocale(LC_ALL, "en_US.UTF-8");
    return;
}

/**
 * @brief Set up the the scren of the game. In addition to creating the main window,
this initializes the screen, turns off echoing, and does the basic setup
needed for curses to do its job.
 * 
 */
void setup_screen(void) {
    int h, w;
    putenv("ESCDELAY=25");
    initscr();
    curs_set(0);
    getmaxyx(stdscr, h, w);
    if (h > 1 && w > 1) {
        setup_term_dimensions(h, w, 1, 1);
    } else if ((h > 1 && h < MIN_TERM_H) || (w > 1 && w < MIN_TERM_W)) {
        cleanup_screen();
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

/**
 * @brief Set up the color pairs necessary for rendering the game in color. This function
is only called if color is supported by the terminal.
 * 
 */
void setup_colors(void) {
    /* Set colors to desired shades if able. */
    #if CHANGE_COLORS
    /* TODO: Alter this so that it pulls from the global w_colors array. */
    if (can_change_color()) {
        for (int i = COLOR_BLACK; i <= COLOR_WHITE; i++) {
            if (i >= COLORS) break;
            init_color(i, colors[i].r, colors[i].g, colors[i].b);
        }
        
    }
    #endif
    /* Initialize color pairs */
    for (int i = COLOR_BLACK; i <= COLOR_WHITE; i++) {
        init_pair(i, i, COLOR_BLACK);
    }
    return;
}

/**
 * @brief The counterpart to setup_screen(). Called at the end of the program, and
used to clean up curses artifacts.
 * 
 */
void cleanup_screen(void) {
    endwin();
    return;
}

/* WINDOW MANAGEMENT FUNCTIONS */

/**
 * @brief Create a new window. Wrapper for curses function newwin.
 * 
 * @param h height.
 * @param w width.
 * @param y y coordinate.
 * @param x x coordinate.
 * @return WINDOW* Pointer to the window struct.
 */
WINDOW* create_win(int h, int w, int y, int x) {
    WINDOW* new_win;
    new_win = newwin(h, w, y, x);
    return new_win;
}

/**
 * @brief Clean up a window by erasing, refreshing, and deleting it.
 * 
 * @param win Window to be cleaned up.
 */
void cleanup_win(WINDOW *win) {
    werase(win);
    wrefresh(win);
    delwin(win);
    return;
}

/**
 * @brief Display the text of a file in a scrollable pad.
 * 
 * @param fname Filename to be displayed.
 */
void display_file_text(const char *fname) {
    FILE *fp;
    WINDOW *new_win;
    int i =0;
    int j = 0;
    int action = A_NONE;
    char *line = NULL;
    size_t len = 0;

    /* Write the file to the window */
    fp = fopen(fname, "r");
    if (fp == NULL)
        return;
    new_win = newpad(MAX_FILE_LEN, term.w);
    while (getline(&line, &len, fp) != -1) {
        mvwprintw(new_win, i++, 0, line);
    }
    free(line);
    fclose(fp);
    /* Handle player input */
    f.mode_map = 0;
    while (1) {
        prefresh(new_win, j, 0, 0, 0, term.h - 1, term.w - 1);

        action = handle_keys();
        switch (action) {
            case A_QUIT:
            case A_HELP:
                cleanup_win(new_win);
                f.update_map = 1;
                f.update_msg = 1;
                f.mode_map = 1;
                return;
            case A_NORTH:
            case A_ASCEND:
                j -= 1;
                break;
            case A_SOUTH:
            case A_DESCEND:
                j += 1;
                break;
        }
        j = max(0, j);
    }
}

/**
 * @brief Display the energy window.
 * 
 */
void display_energy_win(void) {
    WINDOW* new_win;
    char buf[128];
    struct actor *cur_npc = g.player;
    int i = 0;

    new_win = newwin(term.h, term.sb_w, 0, term.sb_x);
    box(new_win, 0, 0);

    snprintf(buf, sizeof(buf), "Player Location: (%d, %d)", cur_npc->x, cur_npc->y);
    mvwprintw(new_win, 1, 1, buf);
    memset(buf, 0, 128);
    snprintf(buf, sizeof(buf), "Camera Origin: (%d, %d)", g.cx, g.cy);
    mvwprintw(new_win, 2, 1, buf);
    memset(buf, 0, 128);
    snprintf(buf, sizeof(buf), "HP: (%d/%d) EN: (%d/%d)", cur_npc->hp, cur_npc->hpmax, cur_npc->energy, 100);
    mvwprintw(new_win, 3, 1, buf);
    memset(buf, 0, 128);
    snprintf(buf, sizeof(buf), "Depth: %d meters", g.depth * 4);
    mvwprintw(new_win, 4, 1, buf);
    memset(buf, 0, 128);
    snprintf(buf, sizeof(buf), "Turn %d", g.turns);
    mvwprintw(new_win, 5, 1, buf);

    memset(buf, 0, 128);
    snprintf(buf, sizeof(buf), "Nearby: ");
    mvwprintw(new_win, 7, 1, buf);
    while (cur_npc != NULL) {
        if (is_visible(cur_npc->x, cur_npc->y) && cur_npc != g.player) {
            memset(buf, 0, 128);
            snprintf(buf, sizeof(buf), "%c ", cur_npc->chr);
            wattron(new_win, COLOR_PAIR(cur_npc->color));
            mvwprintw(new_win, 8 + i, 1, buf);
            wattroff(new_win, COLOR_PAIR(cur_npc->color));
            memset(buf, 0, 128);
            snprintf(buf, sizeof(buf), "%s (%d, %d)", actor_name(cur_npc, 0), cur_npc->x, cur_npc->y);
            mvwprintw(new_win, 8 + i, 3, buf);
            i++;
        }
        cur_npc = cur_npc->next;
    }

    wrefresh(new_win);
}

/**
 * @brief Render a bar.
 * 
 * @param win window to render on.
 * @param cur current value.
 * @param max max value.
 * @param x x coordinate.
 * @param y y coordinate.
 * @param width width.
 * @param full color when full.
 * @param empty color when empty.
 */
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

/**
 * @brief Draw the message window.
 * 
 * @param h height.
 * @param full wehther it is being drawn fullscreen.
 */
void draw_msg_window(int h, int full) {
    int i = 0;
    int x, y;
    struct msg *cur_msg;
    WINDOW *win;

    if (full) {
        win = create_win(term.h, term.msg_w, 0, 0);
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

/**
 * @brief Render a map tile.
 * 
 * @param x x coordinate to render at.
 * @param y y coordinate to render at.
 * @param mx x coordinate of the tile on the map.
 * @param my y coordinate of the tile on the map.
 * @param attr attributes to render with.
 * @return int result of map_putch.
 */
int map_put_tile(int x, int y, int mx, int my, int attr) {
    return map_putch(x, y, g.levmap[mx][my].pt->chr, attr);
}

/**
 * @brief Render an actor.
 * 
 * @param x x coordinate to render at.
 * @param y y coordinate to render at.
 * @param actor actor to be rendered.
 * @param attr attributes to render with.
 * @return int result of map_putch.
 */
int map_put_actor(int x, int y, struct actor *actor, int attr) {
    return map_putch(x, y, actor->chr, attr);
}

/**
 * @brief Output a character to the map window. Wrapper for mvwaddch().
 * 
 * @param x x coordinate to render at.
 * @param y y coordinate to render at.
 * @param chr character to render.
 * @param attr attributes to render with.
 * @return int Result of mvwaddch().
 */
int map_putch(int x, int y, int chr, int attr) {
    int ret;
    wattron(map_win, COLOR_PAIR(attr));
    ret = mvwaddch(map_win, y, x, chr); 
    wattroff(map_win, COLOR_PAIR(attr));
    return ret;
}

/**
 * @brief Outputs a character to the map window. Since curses only supports
 a small set of colors, we cast to an integer and modulo.
 * 
 * @param x x coordinate to render at.
 * @param y y coordinate to render at.
 * @param chr character to render.
 * @param attr attributes to render with.
 * @return int Result of map_putch().
 */
int map_putch_truecolor(int x, int y, int chr, unsigned color) {
    int final_color = color % MAX_COLOR;
    return map_putch(x, y, chr, final_color);
}

/**
 * @brief Erase the map window.
 * 
 */
void clear_map(void) {
    werase(map_win);
}

/**
 * @brief Refresh the map window. Moves the window cursor
 to the player's location.
 * 
 */
void refresh_map(void) {
    wmove(map_win, g.player->y - g.cy, g.player->x - g.cx);
    wrefresh(map_win);
}

/**
 * @brief Handle mouse inputs.
 * 
 * @return Return an action.
 */
int handle_mouse(void) {
    int x, y;   /* Mouse cell */
    int gx, gy; /* Map cell */
    MEVENT event;

    if (getmouse(&event) != OK)
        return A_NONE;
    
    x = event.x;
    y = event.y;
    gx = x + g.cx;
    gy = y - g.cy - term.mapwin_y;
    
    if (event.bstate & BUTTON1_CLICKED) {
        if (y <= term.msg_h - 1 && x <= term.msg_w) {
            return A_FULLSCREEN;
        }
    }

    if (f.mode_look) {
        g.cursor_x = gx;
        g.cursor_y = gy;
    }

    /* Left click to travel. */
    if ((event.bstate & BUTTON1_CLICKED)
        && in_bounds(gx, gy)
        && !is_blocked(gx, gy)
        && is_explored(gx, gy)) {
        g.goal_x = gx;
        g.goal_y = gy;
        f.mode_run = 1;
        return A_NONE;
    }
    /* Right click to examine. */
    if (event.bstate & BUTTON3_CLICKED) {
        look_at(gx, gy);
    }

    return A_NONE;
}

/**
 * @brief Handle key inputs. Blocking.
 * 
 * @return int Return the action taken.
 */
int handle_keys(void) {
    int keycode = getch();
    /* This is a bit more complicated than other input systems,
       since curses picks up character codes, rather than
       keyboard strikes. */
    switch(keycode) {
        case 'H':
            f.mode_run = f.mode_map;
            /* FALLTHRU */
        case 'h':
        case KEY_LEFT:
        case '4':
            return A_WEST;
        case 'J':
            f.mode_run = f.mode_map;
            /* FALLTHRU */
        case 'j':
        case KEY_DOWN:
        case '2':
            return A_SOUTH;
        case 'K':
            f.mode_run = f.mode_map;
            /* FALLTHRU */
        case 'k':
        case KEY_UP:
        case '8':
            return A_NORTH;
        case 'L':
            f.mode_run = f.mode_map;
            /* FALLTHRU */
        case 'l':
        case KEY_RIGHT:
        case '6':
            return A_EAST;
        case 'Y':
            f.mode_run = f.mode_map;
            /* FALLTHRU */
        case 'y':
        case '7':
            return A_NORTHWEST;
        case 'U':
            f.mode_run = f.mode_map;
            /* FALLTHRU */
        case 'u':
        case '9':
            return A_NORTHEAST;
        case 'N':
            f.mode_run = f.mode_map;
            /* FALLTHRU */
        case 'n':
        case '3':
            return A_SOUTHEAST;
        case 'B':
            f.mode_run = f.mode_map;
            /* FALLTHRU */
        case 'b':
        case '1':
            return A_SOUTHWEST;
        case '.':
        case '5':
            return A_REST;
        case 'o':
            return A_OPEN;
        case 'c':
            return A_CLOSE;
        case ';':
            return A_LOOK;
        case KEY_MOUSE:
            return handle_mouse();
        case 'p':
            return A_FULLSCREEN;
        case '>':
            return A_DESCEND;
        case '<':
            return A_ASCEND;
        case ',':
            return A_PICK_UP;
        case ':':
            return A_LOOK_DOWN;
        case 'i':
            return A_INVENT;
        case 'x':
            return A_EXPLORE;
        case '?':
            return A_HELP;
        case 'S':
            return A_SAVE;
        case 'Q':
        case 27:
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