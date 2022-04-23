#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include "register.h"
#include "windows.h"
#include "message.h"

int log_string(const char *, int, va_list);
void wrap_string(char *, int);

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

/* Iterates through a string and inserts newlines in order
   to wrap the text in an aesthetically pleasing manner. */
void wrap_string(char *buf, int max_width) {
    int last_space = 0;
    int w = 0;
    int i = 0;

    while (1) {
        if (buf[i] == '\0') return;
        if (buf[i] == ' ') last_space = i;
        if (w >= max_width - 1) {
            buf[last_space] = '\n';
            w = -1;
        }
        w++;
        i++;
    }
}

/* Unwraps the line breaks in a string. */
char *unwrap_string(char *buf) {
    int i =0;
    while (buf[i] != '\0') {
        if (buf[i] == '\n')
            buf[i] = ' ';
        i++;
    }
    return buf;
}

/* Outputs a string to the log. */
int log_string(const char *format, int attr, va_list arg) {
    char *msgbuf = malloc(MAX_MSG_LEN * sizeof(char));
    int i = 0;
    struct msg *cur_msg;
    struct msg *prev_msg;

    vsnprintf(msgbuf, MAX_MSG_LEN * sizeof(char), format, arg);
    msgbuf[MAX_MSG_LEN - 1] = '\0';

    struct msg *new_msg = malloc(sizeof(struct msg));
    wrap_string(msgbuf, MSG_W);
    new_msg->msg = msgbuf;
    new_msg->turn = g.turns;
    new_msg->attr = attr;
    new_msg->next = NULL;
    new_msg->prev = NULL;
    /* Insert message at the beginning of the message list. */
    if (g.msg_list == (struct msg *) 0) {
        g.msg_list = new_msg;
        g.msg_last = new_msg;
    } else {
        new_msg->next = g.msg_list;
        new_msg->next->prev = new_msg;
        g.msg_list = new_msg;
    }
    /* Loop through list and delete messages that exceed MAX_BACKSCROLL. */
    /* TODO: Theoretically, we could keep a bitfield flag that denotes whether the
       message log has been filled. If it has, we skip iterating through the linked
       list and delete the last message. If not, we iterate. Messy, but would save
       a lot of overhead on every call to log_string(). */
    cur_msg = g.msg_list;
    while (cur_msg != NULL) {
        if (i >= MAX_BACKSCROLL) {
            prev_msg->next = NULL;
            prev_msg = cur_msg;
            cur_msg = cur_msg->next;
            free_msg(prev_msg);
            i++;
            continue;
        }
        i++;
        prev_msg = cur_msg;
        g.msg_last = cur_msg;
        cur_msg = cur_msg->next;
    }
    /* Handle rendering */
    f.update_msg = 1;
    return 0;
}

/* Output a message to the message log with standard formatting. */
int logm(const char *format, ...) {
    int ret;
    va_list arg;

    va_start(arg, format);
    ret = log_string(format, WHITE, arg);
    va_end(arg);
    return ret;
}

/* Output a message to the message log with a color attribute. */
int logma(int attr, const char *format, ...) {
    int ret;
    va_list arg;

    va_start(arg, format);
    ret = log_string(format, attr, arg);
    va_end(arg);
    return ret;
}