/**
 * @file save.c
 * @author Kestrel (kestrelg@kestrelscry.com)
 * @brief Functionality related to saving and restoring the gamestate.
 * @version 1.0
 * @date 2022-05-27
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "message.h"
#include "register.h"
#include "save.h"
#include "ai.h"
#include "invent.h"
#include "actor.h"

void save_game(const char *);
void save_actor(FILE *, struct actor *);
struct actor *load_actor(FILE *, struct actor *);

/**
 * @brief Save the game and immediately exit.
 * 
 */
void save_exit(void) {
    save_game("save.bin");
    exit(0);
}

/**
 * @brief Save the current gamestate to a file.
 * 
 * @param fname The name of the file to save the state to.
 */
void save_game(const char *fname) {
    FILE *fp;

    struct actor *cur_actor = g.player;
    int actor_count = 0;

    fp = fopen(fname, "w");
    if (!fp) {
        logma(MAGENTA, "Save Error: Could not open save file %d.", fname);
        return;
    }

    /* This is a bit of a kludge. Essentially, we have to roll back the current
       turn a bit, so that when the game starts up again the player does not
       lose a turn or get a free turn. If we change the main loop logic at some
       point, we can change this as well. */
    g.player->energy -= 100;
    g.turns -= 1;

    /* Write the global strustart_menuct. */
    fwrite(&g, sizeof(struct global), 1, fp);
    /* Write level map */
    for (int x = 0; x < MAPW; x++) {
        for (int y = 0; y < MAPH; y++) {
            fwrite(&(g.levmap[x][y].pt->id), sizeof(int), 1, fp);
        }
    }
    /* Count actors */
    while (cur_actor != NULL) {
        actor_count++;
        cur_actor = cur_actor->next;
    }
    fwrite(&actor_count, sizeof(int), 1, fp);
    /* Write actors */
    cur_actor = g.player;
    while (cur_actor != NULL) {
        save_actor(fp, cur_actor);
        cur_actor = cur_actor->next;
    }
    fclose(fp);
}

/**
 * @brief Save an actor struct to a file. Called recursively in
 order to save objects contained in the inventory of an actor.
 * 
 * @param fp The name of the file to save the actor to.
 * @param actor The actor struct to be saved.
 */
void save_actor(FILE *fp, struct actor *actor) {
    int item_count;
    struct actor *cur_item = actor->invent;

    /* If some wires get crossed and we end up with an actor that
       refers to itself, immediately kill the process. We don't
       want to fill the user's entire hard drive. */
    if (actor->saved) {
        fclose(fp);
        exit(1);
    }
    actor->saved = 1;

    /* Write the actor, then write each of the actor's components. */
    fwrite(actor, sizeof(struct actor), 1, fp);
    if (actor->name) {
        fwrite(actor->name, sizeof(struct name), 1, fp);
    }
    if (actor->ai) {
        fwrite(actor->ai, sizeof(struct ai), 1, fp);
    }
    if (actor->invent) {
        while (cur_item != NULL) {
            cur_item = actor->invent->next;
            item_count++;
        }
        fwrite(&item_count, sizeof(int), 1, fp);
        cur_item = actor->invent;
        while (cur_item != NULL) {
            save_actor(fp, cur_item);
            cur_item = cur_item->next;
        }
    }
    if (actor->item) {
        fwrite(actor->item, sizeof(struct item), 1, fp);
    }
}

/**
 * @brief Load a previously saved gamestate. The gamestate must have been
 made on the same platform.
 * 
 * @param fname The file to be read.
 */
void load_game(const char *fname) {
    FILE *fp;
    int actor_count;
    int tile_id;
    struct actor *cur_actor;
    struct actor **addr;

    fp = fopen(fname, "r");
    if (!fp) {
        logma(MAGENTA, "Load Error: Could not open save file %d.", fname);
        return;
    }
    /* Read the global struct */
    fread(&g, sizeof(struct global), 1, fp);
    /* Clean up message pointers. We could save the message log fairly easily,
       but it would take up a lot of space, so we don't. */
    g.msg_list = NULL;
    g.msg_last = NULL;
    /* Read level map */
    for (int x = 0; x < MAPW; x++) {
        for (int y = 0; y < MAPH; y++) {
            fread(&tile_id, sizeof(int), 1, fp);
            g.levmap[x][y].pt = &permtiles[tile_id];
            g.levmap[x][y].actor = NULL;
            g.levmap[x][y].item_actor = NULL;
        }
    }
    /* Read actors */
    fread(&actor_count, sizeof(int), 1, fp);
    cur_actor = g.player;
    addr = &g.player;
    for (int i = 0; i < actor_count; i++) {
        cur_actor = load_actor(fp, cur_actor);
        *addr = cur_actor;
        addr = &((*addr)->next);
        push_actor(cur_actor, cur_actor->x, cur_actor->y);
        cur_actor->next = NULL;
        cur_actor = cur_actor->next;
    }
    fclose(fp);
}

/**
 * @brief Load an actor struct from a file.
 * 
 * @param fp The file to be loaded from.
 * @param actor The location in memory to allocate memory for and
 load the actor. Mutated by this function call.
 * @return struct actor* The actor that was loaded.
 */
struct actor *load_actor(FILE *fp, struct actor *actor) {
    int item_count;
    struct actor *cur_item;
    struct actor **addr;

    actor = (struct actor *) malloc(sizeof(struct actor));
    fread(actor, sizeof(struct actor), 1, fp);
    if (actor->name) {
        actor->name = (struct name *) malloc(sizeof(struct name));
        fread(actor->name, sizeof(struct name), 1, fp);
    }
    if (actor->ai) {
        actor->ai = (struct ai *) malloc(sizeof(struct ai));
        fread(actor->ai, sizeof(struct ai), 1, fp);
        actor->ai->parent = actor;
    }
    if (actor->invent) {
        fread(&item_count, sizeof(int), 1, fp);
        cur_item = actor->invent;
        addr = &(actor->invent);
        for (int i = 0; i < item_count; i++) {
            cur_item = load_actor(fp, cur_item);
            *addr = cur_item;
            addr = &((*addr)->next);
            cur_item->next = NULL;
            cur_item = cur_item->next;
        }
    }
    if (actor->item) {
        actor->item = (struct item *) malloc(sizeof(struct item));
        fread(actor->item, sizeof(struct item), 1, fp);
        actor->item->parent = actor;
    }
    actor->saved = 0;
    return actor;
}