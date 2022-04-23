#include <stdio.h>
#include <stdlib.h>

#include "register.h"
#include "message.h"
#include "windows.h"

int write_dumplog(const char *);
void dump_levmap(FILE *);
void dump_messages(FILE *);

void end_game(void) {
    if (!write_dumplog("dumplog.txt")) {
        display_file_text("dumplog.txt");
    }
    exit(0);
}

int write_dumplog(const char *fname) {
    FILE *fp;
    fp = fopen(fname, "w");
    if (!fp) {
        return 1;
    }
    fputs("== Cause of Death ==\n", fp);
    fprintf(fp, "I quit on turn %d.\n", g.turns);
    dump_levmap(fp);
    dump_messages(fp);
    fclose(fp);
    return 0;
}

void dump_levmap(FILE *fp) {
    fputs("\n== Level Map ==\n", fp);
    for (int y = 0; y < MAPH; y++) {
        for (int x = 0; x < MAPW; x++) {
            fputc(g.levmap[x][y].pt->chr, fp);
        }
        fputc('\n', fp);
    }
}

void dump_messages(FILE *fp) {
    struct msg *cur_msg = g.msg_last;
    fputs("\n== Last Recorded Messages ==\n", fp);
    while (cur_msg != NULL) {
        fprintf(fp, "%d | %s\n", cur_msg->turn, cur_msg->msg);
        cur_msg = cur_msg->prev;
    }
}