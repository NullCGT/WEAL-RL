#ifndef AI_H
#define AI_H

struct ai {
    struct actor *parent;
    int hostility;
    /* Various bitfields */
};

void take_turn(struct actor*);

#endif