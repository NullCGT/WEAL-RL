#include <stdlib.h>
#include <locale.h>
#include <signal.h>
#include <stdio.h>
#include <execinfo.h>
#include <unistd.h>
#include <string.h> /* for strcpy */

#include "ai.h"
#include "invent.h"
#include "map.h"
#include "register.h"
#include "windows.h"
#include "message.h"
#include "render.h"
#include "mapgen.h"
#include "random.h"
#include "action.h"
#include "save.h"
#include "creature.h"

void handle_exit(void);
void handle_sigwinch(int);
void new_game(void);

/* Called whenever the program exits. Cleans up the screen and (eventually)
   saves the program state. */
void handle_exit(void) {
    cleanup_screen();
    printf("Freeing message list...\n");
    free_message_list(g.msg_list);
    printf("Freeing actor list...\n");
    free_actor_list(g.player);
    if (term.saved_locale != NULL) {
        printf("Restoring locale...\n");
        setlocale (LC_ALL, term.saved_locale);
        free(term.saved_locale);
    }
    printf("Goodbye!\n");
    return;
}

/* Called when the terminal is resized. */
void handle_sigwinch(int sig) {
    (void) sig;
    cleanup_screen();
    setup_screen();
    f.update_map = 1;
    f.update_fov = 1;
    f.update_msg = 1;
    render_all();
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

void new_game(void) {
    logma(CYAN, "I've arrived at Fort Tarn."); 
    logma(CYAN, "Icicles creep down the concrete crenelations high above, and the wind howls past barbed wire before cutting straight through my jacket.");
    make_level();

    // Create the player
    g.player = spawn_creature(M_HUMAN, -1, -1);
    strcpy(g.player->name, "Player");
    g.player->given_name = 1;
    g.player->unique = 1;

    spawn_creature(M_GLASSWORM, -1, -1);
}

/* Main function. */
int main(void) {
    struct actor *cur_actor;
    FILE *fp;

    /* handle exits and resizes */
    atexit(handle_exit);
    signal(SIGWINCH, handle_sigwinch);
    signal(SIGSEGV, handle_sigsegv);

    // Seed the rng
    rndseed_t();

    // Set up the screen
    setup_screen();

    fp = fopen("save.bin", "r");
    if (fp) {
        fclose(fp);
        load_game("save.bin");
        logma(CYAN, "Welcome back. You ready for this?");
    } else {
        new_game();
    }
    
    /* Main Loop */
    cur_actor = g.player;
    render_all();
    while (1) {
        while (cur_actor != NULL) {
            take_turn(cur_actor);
            cur_actor = cur_actor->next;
            if (cur_actor == NULL)
                cur_actor = g.player;
        }
    }
    exit(0);
    return 0;
}
