#ifndef ACTOR_H
#define ACTOR_H

#define MAXNAMESIZ 20

#define MAX_ATTK 4

#define DM_FIRE 0x0001 /* Fire */
#define DM_ELEC 0x0002 /* Electricity */
#define DM_AIR  0x0004 /* Air */
#define DM_COLD 0x0008 /* Ice */
#define DM_POIS 0x0010 /* Poison */
#define DM_STAB 0x0020 /* stab */
#define DM_CUT  0x0040 /* Slash */
#define DM_BLDG 0x0080 /* Bludgeon */
#define DM_HOLY 0x0100 /* Holy */
#define DM_DARK 0x0200 /* Dark */

struct damage {
    const char *str;
    unsigned char color;
    unsigned long val; 
};

struct tag {
    const char *str;
    unsigned long val;
};

#define MAX_DTYPE 10
#define MAX_TAGS 13

struct attack {
    unsigned char dam_n;
    unsigned char dam_d;
    unsigned char kb;
    unsigned char accuracy;
    /* bitfields */
    unsigned long dtype;
};

struct name {
    char real_name[MAXNAMESIZ];
    char given_name[MAXNAMESIZ];
};

struct actor {
    int chr, tile_offset;
    unsigned char color;
    /* Mutable attributes */
    int x, y;
    int energy;
    int hp, hpmax;
    int weight;
    int speed; /* For creatures, denotes move speed. For items, attack speed. */
    signed char temp_evasion;
    signed char temp_accuracy;
    /* Attack list */
    struct attack attacks[MAX_ATTK];
    /* Components */
    struct name *name;
    struct actor *next;
    struct ai *ai;
    struct actor *invent;
    struct item *item;
    struct equip *equip;
    /* bitfields */
    unsigned long weak;
    unsigned long resist;
    unsigned long tags;
    /* bitflags */
    unsigned int unique : 1;
    unsigned int saved : 1; /* Infinite file write loop prevention. */
    /* 6 free bits */
};

/* Naming bitmasks */
#define NAME_CAP      0x01
#define NAME_THE      0x02
#define NAME_A        0x04
#define NAME_MY       0x08
#define NAME_EQ       0x10

#define is_noatk(x) \
    (!(x.dam_n || x.dam_d))

/* Function Prototypes */
void push_actor(struct actor *, int, int);
struct actor *remove_actor(struct actor *);
void actor_sanity_checks(struct actor *);
char *actor_name(struct actor *, unsigned);
void free_actor(struct actor *);
void free_actor_list(struct actor *);
int in_danger(struct actor *);
const char *describe_health(struct actor *);

extern struct damage dtypes_arr[];
extern struct tag tags_arr[];


#endif