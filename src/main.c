#include <stdbool.h>
#include <curses.h>
#include <menu.h>
#include <stdlib.h>

#include "windows.h"
#include "register.h"
#include "render.h"
#include "mapgen.h"

extern struct global g;

int is_player(struct monster *);
int move_mon(struct monster *, int, int);
void handle_exit(void);
void handle_keys(int);

int is_player(struct monster* mon) {
    return (mon == &g.player);
}

/* Called whenever the program exits. Cleans up the screen and (eventually)
   saves the program state. */
void handle_exit(void) {
    cleanup_screen();
    printf("Saving state...\n");
    printf("Save complete.\n");
    printf("Connection severed.\n");
    return;
}

/* Handle key inputs. */
void handle_keys(int keycode) {
    switch(keycode) {
        case 'Q':
            exit(0);
            break;
        case 'h':
        case KEY_LEFT:
            move_mon(&g.player, -1, 0);
            break;
        case 'j':
        case KEY_DOWN:
            move_mon(&g.player, 0, 1);
            break;
        case 'k':
        case KEY_UP:
            move_mon(&g.player, 0, -1);
            break;
        case 'l':
        case KEY_RIGHT:
            move_mon(&g.player, 1, 0);
            break;
        case 'y':
            move_mon(&g.player, -1, -1);
            break;
        case 'u':
            move_mon(&g.player, 1, -1);
            break;
        case 'n':
            move_mon(&g.player, 1, 1);
            break;
        case 'b':
            move_mon(&g.player, -1, 1);
            break;
        default:
            break;
    }
    return;
}

int move_mon(struct monster* mon, int x, int y) {
    int nx = mon->x + x;
    int ny = mon->y + y;
    if (g.levmap[nx][ny].blocked
        || nx < 0 || ny < 0|| nx >= MAP_WIDTH || ny >= MAP_WIDTH) {
	    return 1;
    }
    mon->x = nx;
    mon->y = ny;
    return 0;
}


/* Main function. */
int main(void) {
    int c;
    WINDOW *main_win;
    WINDOW *sidebar;

    // Exit handling
    atexit(handle_exit);

    /* TODO: While we eventually need to perform a temporary locale switch in order to
       support ascii characters beyond 128, doing so will take some setup. We will
       need to save the current locale so that when the program exits, the old locale
       can be reset. */
    /*
    old_locale = setlocale (LC_ALL, NULL);
    saved_locale = strdup (old_locale);
    if (saved_locale == NULL)
        exit(1);
    setlocale(LC_ALL, "");

    setlocale (LC_ALL, saved_locale);
    free (saved_locale);
    */

    // Set up the screen
    main_win = setup_screen();
    g.map_win = create_win(MAP_HEIGHT, MAP_WIDTH, 1, 1);
    sidebar = create_win(SB_H, SB_W, 0, MAP_WIDTH + 2);
    box(g.map_win, 0, 0);
    box(sidebar, 0, 0);
    mvwprintw(sidebar, 1, 1, "PLNAME");
    wrefresh(sidebar);

    make_level();
    render_map();

    // Create the player
    g.player.x = 20;
    g.player.y = 20;
    g.player.chr = '@';
    g.player.next = NULL;
    
    // Do things
    c = 'M';
    do {
        // Clear all monsters from the screen
        clear_monsters();
        handle_keys(c);
        render_all_monsters();
        wmove(g.map_win, g.player.y, g.player.x);
        wrefresh(g.map_win);
    } while ((c = getch()));

    cleanup_win(main_win);
    cleanup_win(g.map_win);
    cleanup_win(sidebar);
    return 0;
}
