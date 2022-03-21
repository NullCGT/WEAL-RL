#include <curses.h>
#include <stdlib.h>
#include <locale.h>

#include "register.h"
#include "windows.h"
#include "message.h"
#include "render.h"
#include "mapgen.h"
#include "monster.h"
#include "random.h"

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
    printf("Freeing message list...\n");
    free_message_list(g.msg_list);
    printf("Restoring locale...\n");
    if (g.saved_locale != NULL) {
        setlocale (LC_ALL, g.saved_locale);
        free(g.saved_locale);
    }
    printf("Goodbye!\n");
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
        if (y <= MSG_H - 1 && x <= MSG_W) {
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
        case '4':
            move_mon(&g.player, -1, 0);
            break;
        case 'j':
        case KEY_DOWN:
        case '2':
            move_mon(&g.player, 0, 1);
            break;
        case 'k':
        case KEY_UP:
        case '8':
            move_mon(&g.player, 0, -1);
            break;
        case 'l':
        case KEY_RIGHT:
        case '6':
            move_mon(&g.player, 1, 0);
            break;
        case 'y':
        case '7':
            move_mon(&g.player, -1, -1);
            break;
        case 'u':
        case '9':
            move_mon(&g.player, 1, -1);
            break;
        case 'n':
        case '3':
            move_mon(&g.player, 1, 1);
            break;
        case 'b':
        case '1':
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
    if (nx < 0 || ny < 0|| nx >= MAP_W || ny >= MAP_H
        || g.levmap[nx][ny].blocked) {
        if (is_player(mon)) {
            logm("You cannot pass that way.");
        }
	    return 1;
    }
    g.turns++;
    mon->x = nx;
    mon->y = ny;
    /* For testing energy */ 
    mon->energy -= 1;
    if (mon->energy < 0) {
        mon->energy = mon->emax;
    }
    /* This is just temporary. In the future, we can cut down on this
       to prevent excessive map updates. */
    if (is_player(mon)) {
        f.update_map = 1;
    }
    return 0;
}

/* Main function. */
int main(void) {
    int c;

    // Exit handling
    atexit(handle_exit);

    // Seed the rng
    rndseed_t();

    // Set up the screen
    setup_screen();
    logma(COLOR_PAIR(GREEN), "Hello, player. Welcome to the game.");
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
    g.player.energy = 10;
    g.player.emax = 100;
    g.player.actions = test_action;
    g.player.next = NULL;
    g.player.playable = 0;
    
    // Do things
    c = 'M';
    do {
        clear_npcs();
        handle_keys(c);
        /* Conditionally update screen elements */
        if (f.update_msg) {
            draw_msg_window(g.msg_win);
        }
        if (f.update_map) {
            render_map();
        }
        render_all_npcs();
        display_energy_win();
        /* move cursor to player */
        wmove(g.map_win, g.player.y - g.cy, g.player.x - g.cx);
        wrefresh(g.map_win);
    } while ((c = getch()));

    cleanup_win(g.map_win);
    return 0;
}
