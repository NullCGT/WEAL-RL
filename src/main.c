/**
 * @file main.c
 * @author Kestrel (kestrelg@kestrelscry.com)
 * @brief The main loop and functions necessary for running the game.
 * @version 1.0
 * @date 2022-05-27
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <stdlib.h>
#include <locale.h>
#include <signal.h>
#include <stdio.h>
#include <execinfo.h>
#include <unistd.h>
#include <string.h>

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
#include "spawn.h"

void handle_exit(void);
void handle_sigwinch(int);
void new_game(void);
void parse_args(int, const char *[]);

/**
 * @brief Called whenever the program exits. Cleans up the screen and
 frees used memory.
 * 
 */
void handle_exit(void) {
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

/**
 * @brief Handle instances of the terminal being resized. Extremely clumsy.
 * 
 * @param sig Signal.
 */
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


/**
 * @brief Handle segmentation faults and dump a meaningful backtrace.
 This is pulled directly from a stack overflow post by user Todd Gamblin.
   https://stackoverflow.com/questions/77005/how-to-automatically-generate-a-stacktrace-when-my-program-crashes
 * 
 * @param sig Signal.
 */
void handle_sigsegv(int sig) {
    void *array[16];
    int size = backtrace(array, 16);
    (void) sig;
    cleanup_screen();
    fprintf(stderr, "Error: signal %d:\n", sig);
    backtrace_symbols_fd(array, size, STDERR_FILENO);
    exit(1);
}

/**
 * @brief Set up a new game.
 * 
 */
void new_game(void) {
    struct coord c;
    logma(CYAN, "Welcome to Limbo. You are knee-deep in garbage.");
    /* Spawn player */
    if (g.player == NULL) {
        g.player = spawn_creature("human", 0, 0);
        getlogin_r(g.player->name->given_name, sizeof(g.player->name->given_name));
        g.player->unique = 1;
        g.active_attacker = g.player;
        text_entry("What is your name?", g.player->name->given_name, MAXNAMESIZ);
        if (g.player->name->given_name[0] > 'Z')
            g.player->name->given_name[0] = g.player->name->given_name[0] - 32;
    }
    /* Make level */
    make_level();
    c = rand_open_coord();
    /* Put player in a random spot */
    push_actor(g.player, c.x, c.y);
    /* Once we are all done, set up the gui. */
    setup_gui();
}

/**
 * @brief Parse the given arguments
 * 
 * @param argc Number of arguments
 * @param argv Argument array
 */
void parse_args(int argc, const char *argv[]) {
    for (int i = 1; i < argc; i++) {
        if (!strncmp(argv[i], "-X", 2)) {
            g.explore = 1;
        } else if (!strncmp(argv[i], "-D", 2)) {
            g.debug = 1;
        } else if (!strncmp(argv[i], "-u", 2)) {
            snprintf(g.userbuf, sizeof(g.userbuf), "%s", argv[i] + 2);
        }
    }
}

/**
 * @brief Main function
 * 
 * @param argc Number of arguments
 * @param argv Argument array
 * @return int 0
 */
int main(int argc, const char *argv[]) {
    struct actor *cur_actor;
    char buf[MAX_USERSZ + 4] = { '\0' };

    /* handle exits and resizes */
    atexit(handle_exit);
    signal(SIGWINCH, handle_sigwinch);
    signal(SIGSEGV, handle_sigsegv);

    // Parse args
    parse_args(argc, argv);
    if (g.userbuf[0] == '\0')
        getlogin_r(g.userbuf, sizeof(g.userbuf));
    
    /* Build savefile name */
    snprintf(buf, sizeof(buf), "%s.sav", g.userbuf);

    // Seed the rng
    rndseed_t();

    // Set up the screen
    setup_screen();
    title_screen();
    if (file_exists(buf)) {
        load_game(buf);
        logma(CYAN, "Your reverie ends, and you open your eyes. Welcome back.");
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
    cleanup_screen();
    exit(0);
    return 0;
}