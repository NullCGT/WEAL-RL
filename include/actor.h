#ifndef ACTOR_H
#define ACTOR_H

#define MAXNAMESIZ 20

struct name {
    char real_name[MAXNAMESIZ];
    char given_name[MAXNAMESIZ];
};

struct actor {
    int chr, tile_offset, color;
    /* Mutable attributes */
    int x, y;
    int energy;
    int hp, hpmax;
    int weight;
    /* Index into creature list */
    int cindex;
    /* Components */
    struct name *name;
    struct actor *next;
    struct ai *ai;
    struct invent *invent;
    struct item *item;
    /* bitfields */
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

#define ACTMON(id, chr, tile, color, hpmax, weight, unique) \
    M_##id

#define PERMCREATURES \
    ACTMON(HUMAN,     '@', 0x2000, WHITE, 100, 100, 0), \
    ACTMON(GLASSWORM, 'w', 0x2001, RED,     6,  60, 0)

enum permcreaturenum {
    PERMCREATURES,
    M_MAX
};

#undef ACTMON

#define ACTMON(id, chr, tile, color, hpmax, weight, unique) \
    { chr, tile, color, -1, -1, 0, hpmax, hpmax, weight, M_##id, NULL, NULL, NULL, NULL, NULL, unique }


extern struct actor permcreatures[];



#endif