#include <curses.h>
#include <stdlib.h>

#include "windows.h"
#include "register.h"

void free_msg(struct msg *);
int log_string(const char *, int, va_list);

#define MAX_BACKSCROLL 20

void free_msg(struct msg *message) {
    free(message->msg);
    free(message);
    return;
}

void free_message_list(struct msg *cur_msg) {
    struct msg *prev_msg = cur_msg;
    while (cur_msg != (struct msg *) 0) {
        cur_msg = cur_msg->next;
        free_msg(prev_msg);
        prev_msg = cur_msg;
    }
}

int draw_msg_window(WINDOW *win) {
    int ret = 0;
    int i = 0;
    int h, w;
    struct msg *cur_msg;
    struct msg *prev_msg;

    werase(win);
    box(win, 0, 0);
    getmaxyx(win, h, w);
    cur_msg = g.msg_list;
    while (cur_msg != NULL) {
        if (i >= MAX_BACKSCROLL) {
            prev_msg->next = NULL;
            prev_msg = cur_msg;
            cur_msg = cur_msg->next;
            free_msg(prev_msg);
            i++;
            continue;
        } else if (i > h - 3) {
            prev_msg = cur_msg;
            cur_msg = cur_msg->next;
            i++;
            continue;
        }
        wattron(win, cur_msg->attr);
        ret = mvwprintw(win, 1 + i, 1, cur_msg->msg);
        wattroff(win, cur_msg->msg);
        prev_msg = cur_msg;
        cur_msg = cur_msg->next;
        i++;
    }
    wrefresh(win);
    f.update_msg = 0;
    return ret;
}

/* Outputs a string to the log. */
int log_string(const char *format, int attr, va_list arg) {
    char *msgbuf = malloc(128 * sizeof(char));

    vsprintf(msgbuf, format, arg);
    msgbuf[127] = '\0';

    struct msg *new_msg = malloc(sizeof(msg));
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

int logm(const char *format, ...) {
    int ret;
    va_list arg;

    va_start(arg, format);
    ret = log_string(format, A_NORMAL, arg);
    va_end(arg);
    return ret;
}

int logma(int attr, const char *format, ...) {
    int ret;
    va_list arg;

    va_start(arg, format);
    ret = log_string(format, attr, arg);
    va_end(arg);
    return ret;
}

void full_msg_window(void) {
    WINDOW *win;
    win = create_win(MAP_HEIGHT + 2, MAP_WIDTH + 2, 0, 0);
    draw_msg_window(win);
    mvwprintw(win, 0, 0, "[Viewing full message backscroll. Press any key to exit.]");
    wrefresh(win);
    getch();
    cleanup_win(win);
    f.update_map = 1;
    return;
}