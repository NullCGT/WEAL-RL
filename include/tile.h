#ifndef TILE_H
#define TILE_H

#include <wchar.h>
#include "color.h"
#include "actor.h"

/* Attributes are fixed */
struct permtile {
    int id;
    const char *name;
    char chr;
    wchar_t wchr;
    int color;
    int (* func) (struct actor *, int, int);
    /* bitfields */
    unsigned int blocked : 1;
    unsigned int opaque : 1;
    /* 6 free bits */
};

/* Attributes modifiable during runtime. */
struct tile {
    int color;
    struct permtile *pt;
    /* Maintain a pointer to the actor at this location. */
    struct actor *actor;
    struct actor *item_actor;
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
void init_tile(struct tile *, int);
int open_door(struct actor *, int, int);
int close_door(struct actor *, int, int);

/* Tile definitions */
#define PERMTILES \
    FLOOR(FLOOR,      "tile floor",    '.', L'.', 0,         WHITE),  \
    FLOOR(STAIR_DOWN, "stairs down",   '>', L'>', 0,         YELLOW), \
    FLOOR(STAIR_UP,   "stairs up",     '<', L'<', 0,         YELLOW), \
    WALL(WALL,        "concrete wall", '#', L'█', 0,         WHITE),  \
    FLOOR(DOOR_OPEN,  "open door",     '|', L'▒', 0,         CYAN),  \
    WALL(DOOR_CLOSED, "closed door",   '+', L'+', open_door, CYAN)

/* Welcome to Macro Hell */
#define TILE(id, name, chr, wchr, func, color, blocked, opaque) \
    T_##id
#define WALL(id, name, chr, wchr, func, color) \
    T_##id
#define FLOOR(id, name, chr, wchr, func, color) \
    T_##id

enum permtilenum {
    PERMTILES
};

#undef TILE
#undef FLOOR
#undef WALL

#define TILE(id, name, chr, wchr, func, color, blocked, opaque) \
    { T_##id, name, chr, wchr, color, func, blocked, opaque }
#define WALL(id, name, chr, wchr, func, color) \
    { T_##id, name, chr, wchr, color, func, 1, 1 }
#define FLOOR(id, name, chr, wchr, func, color) \
    { T_##id, name, chr, wchr, color, func, 0, 0 }

extern struct permtile permtiles[];

#endif