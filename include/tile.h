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
    unsigned char color;
    int (* func) (struct actor *, int, int);
    int walk_cost;
    int tunnel_cost;
    /* bitfields */
    unsigned int blocked : 1;
    unsigned int opaque : 1;
    /* 6 free bits */
};

/* Attributes modifiable during runtime. */
struct tile {
    unsigned char color;
    struct permtile *pt;
    /* Maintain a pointer to the actor at this location. */
    struct actor *actor;
    struct actor *item_actor;
    /* Heatmaps */
    int player_heat;     /* Player is the goal. */
    int explore_heat;    /* Unexplored areas are the goals. */
    int goal_heat;       /* User-provided location is the goal. */
    int generic_heat;    /* Generic, one-time heatmap use. */
    /* bitfields */
    unsigned int visible : 1;
    unsigned int lit : 1;
    unsigned int explored : 1;
    unsigned int refresh : 1;
    /* 4 free bits */
};

/* Function Prototypes */
int *get_playerh(int, int);
int *get_exploreh(int, int);
int *get_goalh(int, int);
int *get_generich(int, int);
struct tile *init_tile(struct tile *, int);
int open_door(struct actor *, int, int);
int close_door(struct actor *, int, int);

/* Tile definitions */
#define PERMTILES \
    FLOOR(FLOOR,      "cobbled floor",    '.', L'.', 0,         1, 1,  WHITE),  \
    FLOOR(STAIR_DOWN, "stairs down",      '>', L'>', 0,         1, 1,  BRIGHT_YELLOW), \
    FLOOR(STAIR_UP,   "stairs up",        '<', L'<', 0,         1, 1,  BRIGHT_YELLOW), \
    WALL(WALL,        "concrete wall",    '#', L'█', 0,         3, 20, WHITE),  \
    WALL(EARTH,       "unworked stone",   '0', L'#', 0,         3, 20, WHITE), \
    FLOOR(DOOR_OPEN,  "open door",        '|', L'▒', 0,         1, 1, CYAN),  \
    WALL(DOOR_CLOSED, "closed door",      '+', L'+', open_door, 2, 1, CYAN)

/* Welcome to Macro Hell */
#define TILE(id, name, chr, wchr, func, wcost, tcost, color, blocked, opaque) \
    T_##id
#define WALL(id, name, chr, wchr, func, wcost, tcost, color) \
    T_##id
#define FLOOR(id, name, chr, wchr, func, wcost, tcost, color) \
    T_##id

enum permtilenum {
    PERMTILES
};

#undef TILE
#undef FLOOR
#undef WALL

#define TILE(id, name, chr, wchr, func, wcost, tcost, color, blocked, opaque) \
    { T_##id, name, chr, wchr, color, func, wcost, tcost, blocked, opaque }
#define WALL(id, name, chr, wchr, func, wcost, tcost, color) \
    { T_##id, name, chr, wchr, color, func, wcost, tcost, 1, 1 }
#define FLOOR(id, name, chr, wchr, func, wcost, tcost, color) \
    { T_##id, name, chr, wchr, color, func, wcost, tcost, 0, 0 }

extern struct permtile permtiles[];

#endif