#include <stdio.h>
#include <stdlib.h>

#include "message.h"
#include "register.h"
#include "save.h"
#include "ai.h"
#include "invent.h"
#include "actor.h"

void save_game(const char *);
void save_actor(FILE *, struct actor *);
struct actor *load_actor(FILE *, struct actor *);

int save_exit(void) {
    save_game("save.bin");
    exit(0);
    return 0;
}

void save_game(const char *fname) {
    FILE *fp;

    struct actor *cur_actor = g.player;
    int actor_count = 0;

    /* Assume it is the player's turn, and adjust their energy down
       in preparation for when the game is reloaded. */
    g.player->energy -= 100;

    fp = fopen(fname, "w");
    if (!fp) {
        logma(MAGENTA, "Save Error: Could not open save file %d.", fname);
        return;
    }
    /* Write level map */
    fwrite(&g.levmap, sizeof(g.levmap), 1, fp);
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

void save_actor(FILE *fp, struct actor *actor) {
    fwrite(actor, sizeof(struct actor), 1, fp);
    if (actor->ai) {
        fwrite(actor->ai, sizeof(struct ai), 1, fp);
    }
    if (actor->invent) {
        fwrite(actor->invent, sizeof(struct invent), 1, fp);
        for (int i = 0; i < MAX_INVENT_SIZE; i++) {
            if (actor->invent->items[i])
                save_actor(fp, actor->invent->items[i]);
        }
    }
    if (actor->item) {
        fwrite(actor->item, sizeof(struct item), 1, fp);
    }
}

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
    /* Read level map */
    fread(&g.levmap, sizeof(g.levmap), 1, fp);
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

struct actor *load_actor(FILE *fp, struct actor *actor) {
    actor = (struct actor *) malloc(sizeof(struct actor));
    fread(actor, sizeof(struct actor), 1, fp);
    if (actor->ai) {
        fread(actor->ai, sizeof(struct ai), 1, fp);
        actor->ai->parent = actor;
    }
    if (actor->invent) {
        actor->invent = (struct invent *) malloc(sizeof(struct invent));
        fread(actor->invent, sizeof(struct invent), 1, fp);
        for (int i = 0; i < MAX_INVENT_SIZE; i++) {
            if (actor->invent->items[i])
                actor->invent->items[i] = load_actor(fp, actor->invent->items[i]);
            actor->invent->parent = actor;
        }
    }
    if (actor->item) {
        fread(actor->item, sizeof(struct item), 1, fp);
        actor->item->parent = actor;
    }
    return actor;
}