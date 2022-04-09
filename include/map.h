#ifndef MAP_H
#define MAP_H

/* Not a Melty Blood reference, I swear. */
#define MAX_HEAT 999
#define IMPASSABLE MAX_HEAT + 1

int make_visible(int, int);
void magic_mapping(void);
void change_depth(int);
void create_heatmap(void);

#define in_bounds(x, y) \
    (x > 0 && x < MAPW && y > 0 && y < MAPH)
#define is_opaque(x, y) \
    (g.levmap[x][y].opaque)
#define is_blocked(x, y) \
    (g.levmap[x][y].blocked)
#define is_visible(x, y) \
    (g.levmap[x][y].visible)
#define is_explored(x, y) \
    (g.levmap[x][y].explored)
#define is_lit(x, y) \
    (g.levmap[x][y].lit)

#endif