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

#endif