#include <stdlib.h>

#include "register.h"
#include "message.h"
#include "windows.h"

struct monster *create_monster(int id, int level) {
    struct monster *new_mon = malloc(sizeof(struct monster));
    new_mon->level = 1;
    new_mon->monstat = &monstats[id];
    new_mon->max_hp = new_mon->monstat->tough * new_mon->level;
    new_mon->cur_hp = new_mon->max_hp;

    return new_mon;
}

void do_wild_encounter(void) {
    struct monster *wild_mon;
    wild_mon = create_monster(0, 1);
    logm("A wandering %s wants to battle!", wild_mon->monstat->name);
    draw_msg_window(g.msg_win);
    create_popup_win("Wandering monster!", wild_mon->monstat->description);
    return;
}