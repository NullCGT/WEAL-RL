#ifndef REGISTER_H
#define REGISTER_H

#include "color.h"
#include "tile.h"

/* Map and window constants */
#define MAPW 80
#define MAPH 40
#define MIN_TERM_H 80
#define MIN_TERM_W 40

/* Common functions */
#define max(x, y) (((x) > (y)) ? (x) : (y))
#define min(x, y) (((x) < (y)) ? (x) : (y))
#define signum(x) ((x > 0) - (x < 0))

struct actor {
    char name[20];
    int chr;
    int x;
    int y;
    int energy;
    int emax;
    struct action *actions;
    struct actor *next;
    /* bitfields */
    unsigned int playable : 1;
    /* 7 free bits */
};

typedef struct global {
    struct tile levmap[MAPW][MAPH];
    struct actor player; /* Assume player is first NPC */
    struct msg *msg_list;
    struct msg *msg_last;
    int turns;
    int depth;
    int cx;
    int cy;
    int display_heat;
    int prev_action; /* for the moment, only used for runmode */
    char *saved_locale;
} global;

typedef struct bitflags {
    /* Rendering update flags */
    unsigned int update_msg : 1;
    unsigned int update_map : 1;
    unsigned int update_fov : 1;
    /* Mode flags */
    unsigned int mode_explore : 1;
    unsigned int mode_run : 1;
    unsigned int mode_map : 1;
    /* 2 free bits */
} bitflags;

typedef struct terminal {
    int h;
    int w;
 /*
    int mapwin_w;
    int mapwin_h;
    int mapwin_y;
    int msg_w;
    int msg_h;
    int msg_y;
    int sb_w;
    int sb_h;
    int sb_x; */
} terminal;

extern struct global g;
extern struct bitflags f;
extern struct terminal term;

/* Map and window macros */
#define MAPWIN_W (term.w * 2 / 3)
#define MSG_W MAPWIN_W - 2
#define SB_W (term.w - MAPWIN_W)

#define MAPWIN_H min(MAPH, (term.h * 5 / 6))
#define MSG_H (term.h - MAPWIN_H)
#define SB_H MAPWIN_H + MSG_H

#define MAPWIN_Y MSG_H
#define MSG_Y 0
#define SB_Y 0

#define SB_X MAPWIN_W

#endif