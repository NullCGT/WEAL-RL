#include <curses.h>
#include <stdlib.h>
#include <locale.h>

#include "register.h"
#include "windows.h"
#include "render.h"
#include "mapgen.h"
#include "message.h"
#include "monster.h"

int is_player(struct npc *);
int move_mon(struct npc *, int, int);
void handle_exit(void);
void handle_mouse(void);
void handle_keys(int);

int is_player(struct npc* mon) {
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
        case 'w':
            do_wild_encounter();
            break;
        default:
            break;
    }
    return;
}

int move_mon(struct npc* mon, int x, int y) {
    int nx = mon->x + x;
    int ny = mon->y + y;
    if (g.levmap[nx][ny].blocked
        || nx < 0 || ny < 0|| nx >= MAP_WIDTH || ny >= MAP_WIDTH) {
	    return 1;
    }
    g.turns++;
    mon->x = nx;
    mon->y = ny;
    /* logma(COLOR_PAIR(YELLOW), "Moved to (%d, %d)", nx, ny); */
    return 0;
}

/* Main function. */
int main(void) {
    int c;

    // Exit handling
    atexit(handle_exit);

    // Set up the screen
    setup_screen();
    box(g.map_win, 0, 0);
    logma(COLOR_PAIR(GREEN), "Hello, %d. Welcome to the game.", 626);
    wrefresh(g.msg_win);

    make_level();

    struct action *test_action = malloc(sizeof(struct action));
    test_action->desc = "description";
    test_action->name = "test action";
    test_action->func = NULL;
    test_action->next = NULL;

    // Create the player
    g.player.x = 20;
    g.player.y = 20;
    g.player.chr = '@';
    g.player.actions = test_action;
    g.player.next = NULL;
    
    // Do things
    c = 'M';
    do {
        clear_npcs();
        handle_keys(c);
        render_all_npcs();
        /* Conditionally update screen elements */
        if (f.update_msg) {
            draw_msg_window(g.msg_win);
        }
        if (f.update_map) {
            render_map();
        }
        /* move cursor to player */
        wmove(g.map_win, g.player.y, g.player.x);
        wrefresh(g.map_win);
    } while ((c = getch()));

    cleanup_win(g.map_win);
    return 0;
}
