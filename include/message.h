#ifndef MESSAGE_H
#define MESSAGE_H

struct msg {
    char *msg;
    int turn;
    int attr;
    struct msg *next;
};

void free_msg(struct msg *);
void free_message_list(struct msg *);
int logm(const char *, ...);
int logma(int, const char *, ...);

#define MAX_MSG_LEN 256
#define MAX_BACKSCROLL 50

#endif