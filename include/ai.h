#ifndef AI_H
#define AI_H

struct ai {
    struct actor *parent;
    /* Stats */
    int seekdef;
    /* Mutable values */
    int seekcur;
    /* Various bitfields */
    unsigned long faction;
};

struct ai *init_ai(struct actor *);
void take_turn(struct actor *);
struct attack choose_attack(struct actor *, struct actor *);
int is_aware(struct actor *, struct actor *); 

#endif