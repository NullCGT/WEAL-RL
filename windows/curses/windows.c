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
#include "ai.h"
#include "version.h"
#include "save.h"
#include "combat.h"

void wcolor_on(WINDOW *, unsigned char);
void wcolor_off(WINDOW *, unsigned char);
void setup_gui(void);
void setup_locale(void);
void setup_colors(void);
void display_stats(WINDOW *, int *, struct actor *);
void print_dtypes(WINDOW *, int, int, short, unsigned char);
void render_bar(WINDOW*, int, int, int, int, int, int, int);
int handle_mouse(void);

#define MAX_FILE_LEN 200

struct curse_color {
    int r, g, b;
};

#define CHANGE_COLORS 0

WINDOW *map_win;
WINDOW *msg_win;
WINDOW *sb_win;

/* SCREEN FUNCTIONS */

void wcolor_on(WINDOW *win, unsigned char color) {
    wattron(win, COLOR_PAIR(color));
    if (color >= BRIGHT_COLOR)
        wattron(win, A_BOLD);
}

void wcolor_off(WINDOW *win, unsigned char color) {
    wattroff(win, COLOR_PAIR(color));
    if (color >= BRIGHT_COLOR)
        wattroff(win, A_BOLD);
}

void title_screen(void) {
    WINDOW *background;
    struct menu *selector;
    char buf[64];
    int selected = -1;

    background = newwin(term.h, term.w, 0, 0);
    box(background, 0, 0);
    wrefresh(background);

    snprintf(buf, sizeof(buf), "WEAL v%d.%d.%d-%s", 
             VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, RELEASE_TYPE);
    selector = menu_new(buf, term.w / 2 - 10, 1, 24, 7);

    if (file_exists("save.bin")) {
        menu_add_item(selector, 'p', "Continue");
    } else {
        menu_add_item(selector, 'p', "Play");
    }
    menu_add_item(selector, 'd', "View Last Character");
    menu_add_item(selector, 'r', "Records");
    menu_add_item(selector, 'h', "Help");
    menu_add_item(selector, 'q', "Quit");

    while (1) {
        selected = menu_do_choice(selector, 0);
        switch (selected) {
            case 'p':
                menu_destroy(selector);
                cleanup_win(background);
                return;
            case 'd':
                display_file_text("dumplog.txt");
                break;
            case 'r':
                continue;
            case 'h':
                display_file_text("data/text/help.txt");
                break;
            case 'q':
                menu_destroy(selector);
                cleanup_win(background);
                cleanup_screen();
                exit(0);
        }
    }
}

/**
 * @brief Perform the first-time setup for the game's GUI.
 * 
 */
void setup_gui(void) {
    map_win = create_win(term.mapwin_h, term.mapwin_w, term.mapwin_y, 0);
    msg_win = create_win(term.msg_h, term.msg_w, term.msg_y, 0);
    sb_win = create_win(term.sb_h, term.sb_w, 0, term.sb_x);
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
    mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);
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
    for (int i = COLOR_BLACK; i < MAX_COLOR; i++) {
        if (i < BRIGHT_COLOR)
            init_pair(i, i, COLOR_BLACK);
        else
            init_pair(i, i - BRIGHT_COLOR, COLOR_BLACK);
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
    int key = 0;
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

        key = handle_keys();
        switch (key) {
            case 27:
                werase(new_win);
                prefresh(new_win, j, 0, 0, 0, term.h - 1, term.w - 1);
                delwin(new_win);
                f.update_map = 1;
                f.update_msg = 1;
                f.mode_map = 1;
                return;
            case KEY_UP:
            case '8':
                j -= 1;
                break;
            case KEY_DOWN:
            case '2':
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
/* TODO: This leaks memory because it keeps allocating windows without deleting them. Do not use this in its current iteration. */
void display_energy_win(void) {
    char buf[128];
    struct actor *cur_npc = g.player;
    struct attack *cur_attack = get_active_attack();
    int j = 1;

    werase(sb_win);
    //box(sb_win, 0, 0);

    display_stats(sb_win, &j, cur_npc);

    memset(buf, 0, 128);
    snprintf(buf, sizeof(buf), "%s [%d%%%%](%dd%d)", 
             g.active_attacker == g.player ? "Unarmed" : actor_name(g.active_attacker, NAME_CAP), 
             cur_attack->accuracy, cur_attack->dam_n, cur_attack->dam_d);
    mvwprintw(sb_win, j, 1, buf);
    print_dtypes(sb_win, j++, 1 + strlen(buf), cur_attack->dtype, 0);

    memset(buf, 0, 128);
    if (g.depth)
        snprintf(buf, sizeof(buf), "Depth: %d meters", g.depth * 4);
    else
        snprintf(buf, sizeof(buf), "Depth: Surface");
    mvwprintw(sb_win, j++, 1, buf);

    memset(buf, 0, 128);
    snprintf(buf, sizeof(buf), "Turn: %d", g.turns);
    mvwprintw(sb_win, j++, 1, buf);
    j++;

    if (g.target != NULL) {
        display_stats(sb_win, &j, g.target);
        j++;
    }

    wattron(sb_win, A_STANDOUT);
    memset(buf, 0, 128);
    snprintf(buf, sizeof(buf), "Nearby");
    mvwprintw(sb_win, j++, 1, buf);
    wattroff(sb_win, A_STANDOUT);
    while (cur_npc != NULL) {
        if (is_visible(cur_npc->x, cur_npc->y) && cur_npc != g.player) {
            memset(buf, 0, 128);
            snprintf(buf, sizeof(buf), "%c", cur_npc->chr);
            wcolor_on(sb_win, cur_npc->color);
            mvwprintw(sb_win, j, 1, buf);
            wcolor_off(sb_win, cur_npc->color);
            
            if (is_aware(cur_npc, g.player)) {
                wcolor_on(sb_win, BRIGHT_YELLOW);
                mvwprintw(sb_win, j, 2, "!");
                wcolor_off(sb_win, BRIGHT_YELLOW);
            }

            memset(buf, 0, 128);
            snprintf(buf, sizeof(buf), "%s (%d, %d)", actor_name(cur_npc, 0), cur_npc->x, cur_npc->y);
            mvwprintw(sb_win, j++, 4, buf);
        }
        cur_npc = cur_npc->next;
    }

    wrefresh(sb_win);
}

void display_stats(WINDOW *win, int *i, struct actor *actor) {
    char buf[128];
    int max_x, max_y;

    getmaxyx(win, max_y, max_x);
    (void) max_y;

    memset(buf, 0, 128);
    wattron(win, A_STANDOUT);
    if (actor == g.player)
        snprintf(buf, sizeof(buf), "%s", actor_name(actor, NAME_CAP));
    else
        snprintf(buf, sizeof(buf), "%s (target)", actor_name(actor, NAME_CAP));
    mvwprintw(win, *i, 1, buf);
    *i+= 1;
    wattroff(win, A_STANDOUT);

    memset(buf, 0, 128);
    if (actor == g.player)
        snprintf(buf, sizeof(buf), "HP: %d/%d", actor->hp, actor->hpmax);
    else
        snprintf(buf, sizeof(buf), "%s", describe_health(actor));
    wcolor_on(win, RED);
    mvwprintw(win, *i, 1, buf);
    if (actor == g.player)
        render_bar(win, actor->hp, actor->hpmax, 15, *i,
                    max_x - 17, '+', ' ');
    *i += 1;
    wcolor_off(win, RED);

    if (actor == g.player) {
        memset(buf, 0, 128);
        snprintf(buf, sizeof(buf), "MP: %d/%d", actor->hp, actor->hpmax);
        wcolor_on(win, BLUE);
        mvwprintw(win, *i, 1, buf);
        render_bar(win, actor->hp, actor->hpmax, 15, *i,
                    max_x - 17, '+', ' ') ;
        *i += 1;
        wcolor_off(win, BLUE);

        memset(buf, 0, 128);
        snprintf(buf, sizeof(buf), "EN: %d/%d", actor->energy, 100);
        wcolor_on(win, (actor->energy != 0 && actor->energy != 100) ? BRIGHT_GREEN : GREEN);
        render_bar(win, actor->energy, 100, 15, *i,
                    max_x - 17, '+', ' ') ;
        mvwprintw(win, *i, 1, buf);
        *i += 1;
        wcolor_off(win, (actor->energy != 0 && actor->energy != 100) ? BRIGHT_GREEN : GREEN);
    }

    if (actor != g.player) {
        if (is_aware(actor, g.player)) {
            wcolor_on(win, BRIGHT_YELLOW);
            mvwprintw(win, *i, 1, "Tracking");
            wcolor_off(win, BRIGHT_YELLOW);
        } else {
            mvwprintw(win, *i, 1, "Unaware");
        }
        *i += 1;
    }

    mvwprintw(win, *i, 1, "Weak:");
    print_dtypes(win, *i, 9, actor->weak, 1);
    *i += 1;
    mvwprintw(win, *i, 1, "Resist:");
    print_dtypes(win, *i, 9, actor->resist, 1);
    *i += 1;
}

void print_dtypes(WINDOW *win, int y, int x, short dtypes, unsigned char blanks) {
    char buf[16]; /* For a more verbose form later. */

    for (int i = 0; i < MAX_DTYPE; i++) {
        if (dtypes & dtypes_arr[i].val) {
            memset(buf, 0, 16);
            wcolor_on(win, dtypes_arr[i].color);
            snprintf(buf, sizeof(buf), "%c", dtypes_arr[i].str[0] - 32);
            mvwprintw(win, y, x, buf);
            wcolor_off(win, dtypes_arr[i].color);
        } else if (blanks) {
            mvwprintw(win, y, x, "_");
            x++;
        }
    }
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
 * @param full character when full.
 * @param empty character when empty.
 */
void render_bar(WINDOW *win, int cur, int max, int x, int y,
                int width, int full, int empty) {
    int pips = (int) (width * cur / max);
    for (int i = 0; i < width; i++) {
        if (i <= pips) {
            mvwaddch(win, y, x + i, full);
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
        if (y > h - 1) {
            cur_msg = cur_msg->next;
            i++;
            continue;
        }
        wcolor_on(win, cur_msg->attr);
        waddstr(win, cur_msg->msg);
        wcolor_off(win, cur_msg->attr);
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
    int ret;
    if (actor == g.target)
        wattron(map_win, A_UNDERLINE);
    ret = map_putch(x, y, actor->chr, attr);
    if (actor == g.target)
        wattroff(map_win, A_UNDERLINE);
    return ret;
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
    wcolor_on(map_win, attr);
    ret = mvwaddch(map_win, y, x, chr); 
    wcolor_off(map_win, attr);
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
        return 0;
    
    x = event.x;
    y = event.y;
    gx = x + g.cx;
    gy = y - g.cy - term.mapwin_y;
    
    #if 0
    if (event.bstate & BUTTON1_CLICKED) {
        if (y <= term.msg_h - 1 && x <= term.msg_w) {
            return A_FULLSCREEN;
        }
    }
    #endif

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
        return 0;
    }
    /* Right click to examine. */
    if (event.bstate & BUTTON3_CLICKED) {
        look_at(gx, gy);
    }

    return 0;
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
        case KEY_MOUSE:
            return handle_mouse();
        case KEY_UP:
            return '8';
        case KEY_DOWN:
            return '2';
        case KEY_LEFT:
            return '4';
        case KEY_RIGHT:
            return '6';
        default:
            return keycode;
    }
    return 0;
}