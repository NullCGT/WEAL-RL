#include <curses.h>

#define MAP_WIDTH 80
#define MAP_HEIGHT 30

#define SB_W 20
#define SB_H 32

#define MSG_W MAP_WIDTH + 2
#define MSG_H 6
#define MSG_Y MAP_HEIGHT + 2

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

struct msg {
    char *msg;
    int turn;
    int attr;
    struct msg *next;
} msg;

typedef struct global {
    struct tile levmap[MAP_WIDTH][MAP_HEIGHT];
    struct monster player;
    struct msg *msg_list;
    int turns;
    char *saved_locale;
    WINDOW *map_win;
    WINDOW *msg_win;
} global;

typedef struct bitflags {
    unsigned int update_msg : 1;
    unsigned int update_map : 1;
    unsigned int update_sidebar : 1;
    /* 5 free bits */
} bitflags;

extern struct global g;
extern struct bitflags f;
