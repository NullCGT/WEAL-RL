#ifndef ACTOR_H
#define ACTOR_H

struct actor {
    char name[20];
    int chr, tile_offset, color;
    /* Mutable attributes */
    int x, y;
    int energy;
    int hp, hpmax;
    int weight;
    /* Index into creature list */
    int cindex;
    /* Components */
    struct actor *next;
    struct ai *ai;
    struct invent *invent;
    struct item *item;
    /* bitfields */
    unsigned int given_name : 1;
    unsigned int unique : 1;
    /* 7 free bits */
};

/* Function prototypes */
void push_actor(struct actor *, int, int);
struct actor *remove_actor(struct actor *);
void actor_sanity_checks(struct actor *);
int do_attack(struct actor *, struct actor *);
char *actor_name(struct actor *, unsigned);
void free_actor(struct actor *);
void free_actor_list(struct actor *);

/* Naming bitmasks */
#define NAME_CAP      0x01
#define NAME_THE      0x02
#define NAME_A        0x04
#define NAME_MY       0x08

#endif