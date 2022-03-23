#ifndef MESSAGE_H
#define MESSAGE_H

#include "curses.h"

void free_message_list(struct msg *);
void full_msg_window(void);
void draw_msg_window(WINDOW *, int);
int logm(const char *, ...);
int logma(int, const char *, ...);

struct msg {
    char *msg;
    int turn;
    int attr;
    struct msg *next;
};

#endif