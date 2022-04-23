#include <stdlib.h>
#include <locale.h>
#include <signal.h>
#include <stdio.h>
#include <execinfo.h>
#include <unistd.h>

#include "map.h"
#include "register.h"
#include "windows.h"
#include "message.h"
#include "render.h"
#include "mapgen.h"
#include "random.h"
#include "fov.h"
#include "action.h"

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


/* This is pulled directly from a stack overflow post by user Todd Gamblin.
   https://stackoverflow.com/questions/77005/how-to-automatically-generate-a-stacktrace-when-my-program-crashes */
void handle_sigsegv(int sig) {
    (void) sig;
    void *array[10];
    size_t size;
    size = backtrace(array, 10);
    fprintf(stderr, "Error: signal %d:\n", sig);
    backtrace_symbols_fd(array, size, STDERR_FILENO);
    exit(1);
}

/* Main function. */
int main(void) {
    int c;

    /* handle exits and resizes */
    atexit(handle_exit);
    signal(SIGWINCH, handle_sigwinch);
    signal(SIGSEGV, handle_sigsegv);

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
    g.player.tile_offset = 0x2000;
    g.player.energy = 10;
    g.player.emax = 100;
    g.player.next = NULL;
    g.player.playable = 1;

    struct actor test_npc = {
        .name = "Troll",
        .chr = 'T',
        .tile_offset = 0x2001,
        .x = g.player.x + 1,
        .y = g.player.y + 1,
        .energy = 10,
        .emax = 100,
        .actions = NULL,
        .next = NULL,
        .playable = 0
    };
    g.player.next = &test_npc;
    
    /* Main Loop */
    c = A_NONE;
    while (1) {
        clear_npcs();
        execute_action(c);
        /* Conditionally update screen elements */
        if (f.update_msg) {
            draw_msg_window(term.msg_h, 0);
        }
        if (f.update_fov) {
            clear_fov();
            calculate_fov(g.player.x, g.player.y, 7);
            create_heatmap(); /* VERY EXPENSIVE. */
        }
        if (f.update_map) {
            render_map();
        }
        render_all_npcs();
        display_energy_win();
        refresh_map();
        c = get_action(); /* Blocking input occurs here */
    }
    exit(0);
}
