#ifndef ACTOR_H
#define ACTOR_H

struct actor {
    char name[20];
    int chr;
    int tile_offset;
    int x;
    int y;
    int energy;
    struct actor *next;
    struct ai *ai;
};

void push_actor(struct actor *, int, int);
void actor_sanity_checks(struct actor *);
int do_attack(struct actor *, struct actor *);

#endif