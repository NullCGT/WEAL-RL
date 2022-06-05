#ifndef MENU_H
#define MENU_H

#include <curses.h>

struct menu_item {
    const char *text;
    unsigned char index;
    struct menu_item *next;
};

struct menu {
    const char *title;
    struct menu_item *items;
    WINDOW *win;
    unsigned char max;
    int selected;
};

struct menu *menu_new(const char *, int, int, int, int);
void menu_add_item(struct menu *, unsigned char, const char *);
void display_menu(struct menu *);
signed char menu_do_choice(struct menu *, int);
void menu_destroy(struct menu *);

#endif