#ifndef AI_H
#define AI_H

struct ai {
    struct actor *parent;
    /* Stats */
    int max_seek;
    int gx, gy;
    /* Mutable values */
    int seektime;
    /* Various bitfields */
    unsigned long faction;
};

void take_turn(struct actor*);
struct attack choose_attack(struct actor *, struct actor *);

#endif