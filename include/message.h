#ifndef MESSAGE_H
#define MESSAGE_H

void free_msg(struct msg *);
void free_message_list(struct msg *);
int logm(const char *, ...);
int logma(int, const char *, ...);

#define MAX_MSG_LEN 128
#define MAX_BACKSCROLL 50

struct msg {
    char *msg;
    int turn;
    int attr;
    struct msg *next;
};

#endif