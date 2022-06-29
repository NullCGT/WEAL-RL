#ifndef MAP_H
#define MAP_H

/* Not a Melty Blood reference, I swear. */
#define MAX_HEAT 999
#define IMPASSABLE MAX_HEAT + 1

/* Coord struct. May move elsewhere later. */
struct coord {
    int x, y;
};

/* MACROS */

/* bounds */
#define in_bounds(x, y) \
    (x >= 0 && x < MAPW && y >= 0 && y < MAPH)
/* permtile attributes */
#define is_opaque(x, y) \
    (g.levmap[x][y].pt->opaque)
#define is_blocked(x, y) \
    (g.levmap[x][y].pt->blocked)
/* tile attributes */
#define is_visible(x, y) \
    (g.levmap[x][y].visible)
#define is_explored(x, y) \
    (g.levmap[x][y].explored)
#define is_lit(x, y) \
    (g.levmap[x][y].lit)
#define needs_refresh(x, y) \
    (g.levmap[x][y].refresh)

/* lookup */
#define MON_AT(x, y) \
    (g.levmap[x][y].actor)
#define ITEM_AT(x, y) \
    (g.levmap[x][y].item_actor)
#define TILE_AT(x, y) \
    (g.levmap[x][y].pt->id)

/* Function Prototypes */
struct coord get_direction(const char *);
int make_visible(int, int);
struct coord rand_open_coord(void);
int magic_mapping(void);
int ascend(void);
int descend(void);
int change_depth(int);
void do_heatmaps(void);

#endif