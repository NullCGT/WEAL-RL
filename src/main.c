#include <curses.h>
#include <stdlib.h>
#include <locale.h>

#include "windows.h"
#include "register.h"
#include "render.h"
#include "mapgen.h"
#include "message.h"

int is_player(struct monster *);
int move_mon(struct monster *, int, int);
void handle_exit(void);
void handle_mouse(void);
void handle_keys(int);

int is_player(struct monster* mon) {
    return (mon == &g.player);
}

/* Called whenever the program exits. Cleans up the screen and (eventually)
   saves the program state. */
void handle_exit(void) {
    cleanup_screen();
    free_message_list(g.msg_list);
    if (g.saved_locale != NULL) {
        setlocale (LC_ALL, g.saved_locale);
        free(g.saved_locale);
    }
    printf("Saving state...\n");
    printf("Save complete.\n");
    printf("Connection severed.\n");
    return;
}

/* handle mouse inputs */
void handle_mouse() {
    int x, y;
    MEVENT event;

    if (getmouse(&event) != OK)
        return;
    
    x = event.x;
    y = event.y;
    
    if (event.bstate & BUTTON1_CLICKED) {
        if (y >= MSG_Y && x <= MSG_W) {
            full_msg_window();
        }
    }
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
        case 'p':
            full_msg_window();
            break;
        case KEY_MOUSE:
            handle_mouse();
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
    g.turns++;
    mon->x = nx;
    mon->y = ny;
    logma(COLOR_PAIR(YELLOW), "Moved to (%d, %d)", nx, ny);
    return 0;
}

/* Main function. */
int main(void) {
    int c;
    WINDOW *sidebar;

    // Exit handling
    atexit(handle_exit);

    // Set up the screen
    setup_screen();
    sidebar = create_win(SB_H, SB_W, 0, MAP_WIDTH + 2);
    box(g.map_win, 0, 0);
    box(sidebar, 0, 0);
    mvwprintw(sidebar, 1, 1, "PLNAME");
    logma(COLOR_PAIR(GREEN), "Hello, %d. Welcome to the game.", 626);
    wrefresh(sidebar);
    wrefresh(g.msg_win);

    make_level();

    // Create the player
    g.player.x = 20;
    g.player.y = 20;
    g.player.chr = '@';
    g.player.next = NULL;
    
    // Do things
    c = 'M';
    do {
        clear_monsters();
        handle_keys(c);
        render_all_monsters();
        /* Conditionally update screen elements */
        if (f.update_msg) {
            draw_msg_window(g.msg_win);
        }
        if (f.update_map) {
            render_map();
        }
        if (f.update_sidebar) {
            mvwprintw(sidebar, 2, 1, "T%d", g.turns);
            wrefresh(sidebar);
        }
        /* move cursor to player */
        wmove(g.map_win, g.player.y, g.player.x);
        wrefresh(g.map_win);
    } while ((c = getch()));

    cleanup_win(g.map_win);
    cleanup_win(sidebar);
    return 0;
}
