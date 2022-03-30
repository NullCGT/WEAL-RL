#include <stdlib.h>
#include <stdio.h>

#include "register.h"
#include "windows.h"
#include "message.h"

int log_string(const char *, int, va_list);

/* Free an individual message. */
void free_msg(struct msg *message) {
    free(message->msg);
    free(message);
    return;
}

/* Free all messages in the linked list of messages. */
void free_message_list(struct msg *cur_msg) {
    struct msg *prev_msg = cur_msg;
    while (cur_msg != (struct msg *) 0) {
        cur_msg = cur_msg->next;
        free_msg(prev_msg);
        prev_msg = cur_msg;
    }
}

/* Outputs a string to the log. */
int log_string(const char *format, int attr, va_list arg) {
    char *msgbuf = malloc(MAX_MSG_LEN * sizeof(char));

    vsnprintf(msgbuf, MAX_MSG_LEN * sizeof(char), format, arg);
    msgbuf[MAX_MSG_LEN - 1] = '\0';

    struct msg *new_msg = malloc(sizeof(struct msg));
    new_msg->msg = msgbuf;
    new_msg->turn = 0;
    new_msg->attr = attr;
    new_msg->next = NULL;

    if (g.msg_list == (struct msg *) 0) {
        g.msg_list = new_msg;
    } else {
        new_msg->next = g.msg_list;
        g.msg_list = new_msg;
    }
    f.update_msg = 1;
    return 0;
}

/* Output a message to the message log. */
int logm(const char *format, ...) {
    int ret;
    va_list arg;

    va_start(arg, format);
    ret = log_string(format, WHITE, arg);
    va_end(arg);
    return ret;
}

/* Output a message to the message log with special
   attributes, such as color or underlining. */
int logma(int attr, const char *format, ...) {
    int ret;
    va_list arg;

    va_start(arg, format);
    ret = log_string(format, attr, arg);
    va_end(arg);
    return ret;
}