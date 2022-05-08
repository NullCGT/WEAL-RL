#ifndef CREATURE_H
#define CREATURE_H

struct permcreature {
    /* Immutable attributes */
    int id;
    const char *name;
    int chr;
    int tile_offset;
    int color;
    int base_hp;
    int weight;
    /* Component */
    struct ai *ai;
    /* Mutable status */
    int num_encounterd;
    int num_slain;
    int num_saved;
    unsigned long information;
};

/* Function prototypes */
void init_creature(struct actor *, int);
struct actor *spawn_creature(int, int, int);

#define MON(id, name, chr, tile, color, base_hp, weight, ai) \
    M_##id

#define PERMCREATURES \
    MON(HUMAN,     "human",     '@', 0x2000, WHITE, 100, 120, NULL), \
    MON(GLASSWORM, "glassworm", 'w', 0x2001, RED,   6, 60, NULL)

enum permcreaturenum {
    PERMCREATURES,
    M_MAX
};

#undef MON

#define MON(id, name, chr, tile, color, base_hp, weight, ai) \
    { M_##id, name, chr, tile, color, base_hp, weight, ai, \
        0, 0, 0, 0 }

extern struct permcreature permcreatures[];

#endif