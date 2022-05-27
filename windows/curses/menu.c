#include <curses.h>
#include <stdlib.h>
#include <string.h>

#include "register.h"
#include "windows.h"
#include "menu.h"

struct menu *menu_new(const char *title) {
    struct menu *new_menu = malloc(sizeof(struct menu));
    new_menu->title = title;
    new_menu->max = 0;
    new_menu->selected = 0;
    new_menu->items = NULL;
    new_menu->win = create_win(term.h, term.w, 0, 0);
    werase(new_menu->win);
    f.mode_map = 0;
    wrefresh(new_menu->win);
    return new_menu;
}

void menu_add_item(struct menu *menu, unsigned char index, const char *text) {
    struct menu_item *cur = menu->items;
    struct menu_item *prev = menu->items;

    struct menu_item *new_item = malloc(sizeof(struct menu_item));
    new_item->index = index;
    new_item->text = text;
    new_item->next = NULL;

    while (cur != NULL) {
        prev = cur;
        cur = cur->next;
    }
    if (prev) prev->next = new_item;
    else menu->items = new_item;

    menu->max += 1;
}

void display_menu(struct menu *menu) {
    struct menu_item *cur = menu->items;
    char itembuf[128];
    int index = 0;

    werase(menu->win);
    wattron(menu->win, A_UNDERLINE);
    sprintf(itembuf, "%s\n\n", menu->title);
    waddstr(menu->win, itembuf);
    wattroff(menu->win, A_UNDERLINE);
    
    while (cur != NULL) {
        memset(itembuf, 0, 128);
        sprintf(itembuf, " %c) %s\n", cur->index, cur->text);
        if (index == menu->selected)
            wattron(menu->win, A_REVERSE);
        waddstr(menu->win, itembuf);
        wattroff(menu->win, A_REVERSE);
        cur = cur->next;
        index++;
    }
}

signed char menu_do_choice(struct menu *menu, int can_quit) {
    struct menu_item *cur_item = menu->items;

    while (1) {
        display_menu(menu);
        wrefresh(menu->win);
        int input = getch();

        if (input == 27 && can_quit) {
            return -1;
        } else if (input == KEY_UP || input == '8') {
            menu->selected = max(0, menu->selected - 1);
        } else if (input == KEY_DOWN || input == '2') {
            menu->selected = min(menu->max - 1, menu->selected + 1);
        } else if (input == KEY_ENTER || input == '\n' || input == '\r') {
            for (int i = 0; i < menu->selected; i++) {
                cur_item = cur_item->next;
            }
            return cur_item->index;
        } else if (input >= 'a' && input <= 'z') {
            return input;
        }
    }
}

void menu_destroy(struct menu *menu) {
    struct menu_item *cur = menu->items;
    struct menu_item *prev = menu->items;

    while (cur != NULL) {
        cur = cur->next;
        free(prev);
        prev = cur;
    }
    cleanup_win(menu->win);
    free(menu);
    f.update_map = 1;
    f.update_msg = 1;
    f.mode_map = 1;
}