#ifndef ACTOR_H
#define ACTOR_H

struct actor {
    char name[20];
    int chr, tile_offset;
    /* Mutable attributes */
    int x, y;
    int energy;
    int hp, hpmax;
    /* Components */
    struct actor *next;
    struct ai *ai;
    struct invent *invent;
    struct item *item;
};

void push_actor(struct actor *, int, int);
struct actor *remove_actor(struct actor *);
void actor_sanity_checks(struct actor *);
int do_attack(struct actor *, struct actor *);
void free_actor(struct actor *);
void free_actor_list(struct actor *);

#endif