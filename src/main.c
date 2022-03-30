#include <stdlib.h>
#include <locale.h>
#include <signal.h>

#include "map.h"
#include "register.h"
#include "windows.h"
#include "message.h"
#include "render.h"
#include "mapgen.h"
#include "monster.h"
#include "random.h"
#include "fov.h"
#include "action.h"

int move_mon(struct npc *, int, int);
void handle_exit(void);
void handle_sigwinch(int);

/* Called whenever the program exits. Cleans up the screen and (eventually)
   saves the program state. */
void handle_exit(void) {
    cleanup_screen();
    printf("Freeing message list...\n");
    free_message_list(g.msg_list);
    if (g.saved_locale != NULL) {
        printf("Restoring locale...\n");
        setlocale (LC_ALL, g.saved_locale);
        free(g.saved_locale);
    }
    printf("Goodbye!\n");
    return;
}

/* Called when the terminal is resized. */
/* TODO: Respond gracefully. */
void handle_sigwinch(int sig) {
    (void) sig;
    #if 0
    cleanup_screen();
    printf("Do not resize the program while running.\n");
    exit(0);
    #endif
    return;
}

/* Main function. */
int main(void) {
    int c;

    /* handle exits and resizes */
    atexit(handle_exit);
    signal(SIGWINCH, handle_sigwinch);

    // Seed the rng
    rndseed_t();

    // Set up the screen
    setup_screen();
    logma(CYAN, "I've arrived at Fort Tarn."); 
    logma(CYAN, "Icicles creep down the concrete crenelations high above, and the wind howls past barbed wire before cutting straight through my jacket.");

    // Create the map
    make_level();

    // Create the player
    g.player.x = 20;
    g.player.y = 20;
    g.player.chr = '@';
    g.player.energy = 10;
    g.player.emax = 100;
    g.player.next = NULL;
    g.player.playable = 0;
    
    /* Main Loop */
    c = A_NONE;
    while (TRUE) {
        clear_npcs();
        execute_action(c);
        /* Conditionally update screen elements */
        if (f.update_msg) {
            draw_msg_window(MSG_H, 0);
        }
        if (f.update_fov) {
            clear_fov();
            calculate_fov(g.player.x, g.player.y, 7);
        }
        if (f.update_map) {
            render_map();
        }
        render_all_npcs();
        display_energy_win();
        refresh_map();
        c = handle_keys();
    }
    exit(0);
}
