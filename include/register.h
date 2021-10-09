#include <curses.h>

#define MAP_WIDTH 80
#define MAP_HEIGHT 30
#define SB_W 20
#define SB_H 32

struct monster {
    char name[20];
    int chr;
    int x;
    int y;
    struct monster *next;
} monster;

struct tile {
    int chr;
    int blocked;
} tile;

typedef struct global {
    struct tile levmap[MAP_WIDTH][MAP_HEIGHT];
    struct monster player;
    WINDOW *map_win;
} global;

extern struct global g;
