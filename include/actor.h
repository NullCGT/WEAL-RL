#ifndef ACTOR_H
#define ACTOR_H

#define MAXNAMESIZ 20

#define MAX_ATTK 4

#define DM_FIRE 0x00000001 /* Fire */
#define DM_ELEC 0x00000002 /* Electricity */
#define DM_AIR  0x00000004 /* Air */
#define DM_ICE  0x00000008 /* Ice */
#define DM_POIS 0x00000010 /* Poison */
#define DM_STAB 0x00000020 /* Pierce */
#define DM_CUT  0x00000040 /* Slash */
#define DM_BLDG 0x00000080 /* Bludgeon */
#define DM_HOLY 0x00000100 /* Holy */
#define DM_DARK 0x00000200 /* Dark */

struct attack {
    unsigned char dam_n;
    unsigned char dam_d;
    unsigned char kb;
    unsigned char timeout;
    /* bitfields */
    unsigned short dtype;
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
    /* Index into creature list */
    int cindex;
    /* Attack list */
    struct attack attacks[MAX_ATTK];
    /* Components */
    struct name *name;
    struct actor *next;
    struct ai *ai;
    struct actor *invent;
    struct item *item;
    /* bitfields */
    unsigned short weak;
    unsigned short resist;
    unsigned int unique : 1;
    unsigned int saved : 1; /* Infinite file write loop prevention. */
    /* 7 free bits */
};

/* Function prototypes */
void push_actor(struct actor *, int, int);
struct actor *remove_actor(struct actor *);
void actor_sanity_checks(struct actor *);
char *actor_name(struct actor *, unsigned);
void free_actor(struct actor *);
void free_actor_list(struct actor *);

/* Naming bitmasks */
#define NAME_CAP      0x01
#define NAME_THE      0x02
#define NAME_A        0x04
#define NAME_MY       0x08

#define ATKS(a1, a2, a3, a4) \
    {a1, a2, a3, a4}

#define ATK(dam_n, dam_d, kb, dtype) \
    {dam_n, dam_d, kb, 0, dtype}

#define NO_ATK \
    {0, 0, 0, 0, 0}

#define is_noatk(x) \
    (!(x.dam_n || x.dam_d))

#define ACTOR(id, chr, tile, color, hpmax, weight, attacks, weakness, resist) \
    M_##id

#define PERMCREATURES \
    ACTOR(HUMAN,     '@', 0x2000, WHITE, 100, 100, \
            ATKS(ATK(1, 6, 0, DM_BLDG), NO_ATK, NO_ATK, NO_ATK), \
            0, 0), \
    ACTOR(ZOMBIE, 'Z', 0x2001, GREEN,      6, 60, \
            ATKS(ATK(1, 6, 0, DM_BLDG), ATK(1, 3, 0, DM_STAB), NO_ATK, NO_ATK), \
            DM_FIRE | DM_HOLY, DM_POIS)

#define PERMITEMS \
    ACTOR(LONGSWORD,    '/', 0x2000, WHITE, 10, 5, \
            ATKS(ATK(1, 6, 0, DM_CUT), NO_ATK, NO_ATK, NO_ATK), \
            DM_BLDG, DM_CUT | DM_STAB)

enum permcreaturenum {
    PERMCREATURES,
    M_MAX
};

enum permitems {
    PERMITEMS,
    I_MAX,
};

#undef ACTOR

#define ACTOR(id, chr, tile, color, hpmax, weight, attacks, weakness, resist) \
    { chr, tile, color, -1, -1, 0, hpmax, hpmax, weight, M_##id, attacks, NULL, NULL, NULL, NULL, NULL, weakness, resist, 0, 0 }


extern struct actor permcreatures[];
extern struct actor permitems[];



#endif