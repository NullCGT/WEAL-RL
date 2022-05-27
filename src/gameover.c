/**
 * @file gameover.c
 * @author Kestrel (kestrelg@kestrelscry.com)
 * @brief Functions called when the game has ended, whether due to a loss or a win.
 * @version 1.0
 * @date 2022-05-26
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <stdio.h>
#include <stdlib.h>

#include "register.h"
#include "message.h"
#include "windows.h"

int write_dumplog(const char *, int);
void dump_killer(FILE *);
void dump_levmap(FILE *);
void dump_messages(FILE *);
void dump_inventory(FILE *);

/**
 * @brief End the game.
 * 
 * @param winner Whether the game was won or lost.
 1 if the game was won.
 0 if the game was lost.
 */
void end_game(int winner) {
    if (!write_dumplog("dumplog.txt", winner)) {
        display_file_text("dumplog.txt");
    }
    exit(0);
}

/**
 * @brief Write a dumplog to a file.
 * 
 * @param fname The name of the file to dump the log to.
 * @param winner Whether the game was lost.
 1 if the game was won.
 0 if the game was lost.
 * @return int Whether the file was successfully opened.
 1 if we could not acquire a file pointer.
 0 if the file was opened successfully.
 */
int write_dumplog(const char *fname, int winner) {
    FILE *fp;
    fp = fopen(fname, "w");
    if (!fp) {
        return 1;
    }
    fputs("== Final Statics ==\n", fp);
    if (winner) {
        fprintf(fp, "I won on turn %d.\n", g.turns);
    } else if (g.killer) {
        fprintf(fp, "I was killed by %s on turn %d.\n", actor_name(g.killer, NAME_A), g.turns);
    } else {
        fprintf(fp, "I quit on turn %d.\n", g.turns);
    }
    fprintf(fp, "I had %d health, with a maximum of %d.\n", g.player->hp, g.player->hpmax);

    if (g.killer) {
        dump_killer(fp);
    }

    dump_levmap(fp);
    dump_messages(fp);
    dump_inventory(fp);
    fclose(fp);
    return 0;
}

/**
 * @brief Dump information about the creature that killed the player.
 * 
 * @param fp The dumplog file.
 */
void dump_killer(FILE *fp) {
    struct actor *cur = g.killer->invent;

    fputs("\n== Killer Statistics ==\n", fp);
    fprintf(fp, "Name: %s\n", actor_name(g.killer, NAME_CAP));
    fprintf(fp, "HP: (%d/%d)\n", g.killer->hp, g.killer->hpmax);

    if (cur != NULL) {
        fprintf(fp, "Possessions:\n");
    }
    while (cur != NULL) {
        fprintf(fp, " %s\n", actor_name(cur, NAME_CAP | NAME_A));
        cur = cur->next;
    }
}

/**
 * @brief Dump a textual representation of the current level.
 * 
 * @param fp The dumplog file.
 */
void dump_levmap(FILE *fp) {
    fputs("\n== Level Map ==\n", fp);
    for (int y = 0; y < MAPH; y++) {
        for (int x = 0; x < MAPW; x++) {
            if (g.levmap[x][y].actor)
                fputc(g.levmap[x][y].actor->chr, fp);
            else if (g.levmap[x][y].item_actor)
                fputc(g.levmap[x][y].item_actor->chr, fp);
            else
                fputc(g.levmap[x][y].pt->chr, fp);
        }
        fputc('\n', fp);
    }
}

/**
 * @brief Dump all messages contained in the game's memory.
 * 
 * @param fp The dumplog file.
 */
void dump_messages(FILE *fp) {
    struct msg *cur_msg = g.msg_last;
    fputs("\n== Last Recorded Messages ==\n", fp);
    while (cur_msg != NULL) {
        fprintf(fp, "%d | %s\n", cur_msg->turn, cur_msg->msg);
        cur_msg = cur_msg->prev;
    }
}

/**
 * @brief Dump a record of all items in the player's inventory.
 * 
 * @param fp The dumplog file.
 */
void dump_inventory(FILE *fp) {
    struct actor *cur = g.player->invent;

    fputs("\n== Inventory ==\n", fp);
    if (!cur) {
        fprintf(fp, "A bit of dust.");
        return;
    }
    while (cur != NULL) {
        fprintf(fp, "%s\n", actor_name(cur, NAME_CAP | NAME_A));
        cur = cur->next;
    }

}