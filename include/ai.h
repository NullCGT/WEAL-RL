#ifndef AI_H
#define AI_H

struct ai {
    struct actor *parent;
    /* Stats */
    int max_seek;
    /* Mutable values */
    int seektime;
    /* Various bitfields */
    unsigned long faction;
};

void init_ai(struct actor *);
void take_turn(struct actor *);
struct attack choose_attack(struct actor *, struct actor *);

#endif