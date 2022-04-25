#ifndef TILE_H
#define TILE_H

#include <wchar.h>
#include "color.h"

/* Attributes are fixed */
struct permtile {
    int id;
    const char *name;
    char chr;
    wchar_t wchr;
    int color;
    /* bitfields */
    unsigned int blocked : 1;
    unsigned int opaque : 1;
    /* 6 free bits */
};

/* Attributes modifiable during runtime. */
struct tile {
    int chr;
    int color;
    struct permtile *pt;
    /* Maintain a pointer to the actor at this location. */
    struct actor *actor;
    /* Heatmaps */
    int player_heat;
    int explore_heat;
    /* bitfields */
    unsigned int visible : 1;
    unsigned int lit : 1;
    unsigned int explored : 1;
    /* 4 free bits */
};

/* Function prototypes */
void init_tile(struct tile *intile, int tindex);

/* Tile definitions */
#define PERMTILES \
    FLOOR(FLOOR,      "tile floor",    '.', L'.', WHITE), \
    FLOOR(STAIR_DOWN, "stairs down",   '>', L'>', YELLOW), \
    FLOOR(STAIR_UP,   "stairs up",     '<', L'<', YELLOW), \
    WALL(WALL,        "concrete wall", '#', L'█', WHITE)

/* Welcome to Macro Hell */
#define TILE(id, name, chr, wchr, color, blocked, opaque) \
    T_##id
#define WALL(id, name, chr, wchr, color) \
    T_##id
#define FLOOR(id, name, chr, wchr, color) \
    T_##id

enum permtilenum {
    PERMTILES
};

#undef TILE
#undef FLOOR
#undef WALL

#define TILE(id, name, chr, wchr, color, blocked, opaque) \
    { T_##id, name, chr, wchr, color, blocked, opaque }
#define WALL(id, name, chr, wchr, color) \
    { T_##id, name, chr, wchr, color, 1, 1 }
#define FLOOR(id, name, chr, wchr, color) \
    { T_##id, name, chr, wchr, color, 0, 0 }

extern struct permtile permtiles[];

#endif