/**
 * @file invent.c
 * @author Kestrel (kestrelg@kestrelscry.com)
 * @brief Inventory and item-related functionality.
 * @version 1.0
 * @date 2022-05-27
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <stdlib.h>

#include "actor.h"
#include "invent.h"
#include "windows.h"
#include "message.h"
#include "menu.h"
#include "register.h"

struct actor *win_pick_invent(void);
int win_use_item(struct actor *);

/**
 * @brief Allocate memory for the item component of a given actor
 * 
 * @param actor The parent of the new item component.
 */
void init_item(struct actor *actor) {
    struct item *new_item = (struct item *) malloc(sizeof(struct item));
    new_item->parent = actor;
    new_item->letter = 'a';
    new_item->quan = 1;
    actor->item = new_item;
}

/**
 * @brief Perform an action with an item picked from the inventory.
 * 
 * @return The costi n energy of the performed action. 
 */
int display_invent(void) {
    int ret = 0;
    struct actor *item;
    item = win_pick_invent();
    if (item)
        ret = win_use_item(item);
    return ret;
}

/**
 * @brief Add an item to a given inventory. Letters are "sticky," in that
if you drop an item, it should have the same letter when you
pick it back up. If there is a collision, the letter is reassigned.
Returns 1 if item was successfully added.
Returns 0 on failure.
 * 
 * @param creature The creature whose inventory is being appended to.
 * @param item The actor being added to the inventory.
 * @return int Success.
0 if unsuccessful.
1 if successful.
 */
int add_to_invent(struct actor *creature, struct actor *item) {
    struct actor *cur = creature->invent;
    struct actor *prev = creature->invent;
    int found_spot = 0;

    char letters[26] = "abcdefghijklmnopqrstuvwyz";
    
    /* Transcend list, removing used letters. */
    while (cur != NULL) {
        prev = cur;
        letters[cur->item->letter - 'a'] = 0;
        cur = cur->next;
    }
    /* Reassign letter if necessary. */
    if (!letters[item->item->letter - 'a']) {
        for (int i = 0; i < 26; i++) {
            if (letters[i]) {
                item->item->letter = letters[i];
                found_spot = 1;
                break;
            }
        }
        if (!found_spot) return 0;
    }
    /* Insert the item */
    if (prev != NULL) {
        prev->next = item;
    } else {
        creature->invent = item;
    }
    return 1;
}

/**
 * @brief Set up the inventory menu and choose an item.
 * 
 * @return struct actor* The item chosen from the inventory menu.
 */
struct actor *win_pick_invent(void) {
    struct menu *selector;
    struct actor *cur = g.player->invent;
    int selected = -1;

    if (cur == NULL) {
        logm("I am not carrying anything.");
        return NULL;
    }

    selector = menu_new("Inventory");
    while (cur != NULL) {
        menu_add_item(selector, cur->item->letter, actor_name(cur, 0));
        cur = cur->next;
    }
    while (1) {
        selected = menu_do_choice(selector, 1);
        if (selected == -1) {
            cur = NULL;
            break;
        }
        cur = g.player->invent;
        while (cur != NULL) {
            if (cur->item->letter == selected) {
                menu_destroy(selector);
                return cur;
            }
            cur = cur->next;
        }
    }
    menu_destroy(selector);
    return cur;
}

/**
 * @brief Display the item use menu.
 * 
 * @param item The item to be used.
 * @return int The energy cost of the action performed with the
 item.
 */
int win_use_item(struct actor *item) {
    struct menu *selector;
    int selected = -1;

    selector = menu_new(actor_name(item, NAME_A | NAME_CAP));
    menu_add_item(selector, 'd', "drop");
    menu_add_item(selector, 'w', "wield");
    menu_add_item(selector, 'p', "put on");

    while (1) {
        selected = menu_do_choice(selector, 1);
        switch (selected) {
            case 'd':
                logm("Dropping %s!", actor_name(item, NAME_THE));
                menu_destroy(selector);
                return 0;
            case 'w':
                logm("Wielding %s!", actor_name(item, NAME_THE));
                menu_destroy(selector);
                return 0;
            case 'p':
                logm("Putting on %s!", actor_name(item, NAME_THE));
                menu_destroy(selector);
                return 0;
            case -1:
                menu_destroy(selector);
                return 0;
        }
    }
    menu_destroy(selector);
    return 0;
}