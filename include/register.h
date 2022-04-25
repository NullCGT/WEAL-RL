#ifndef REGISTER_H
#define REGISTER_H

#include "actor.h"
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

/* Func Proto */
void setup_term_dimensions(int, int, int, int);

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
    int mapwin_w;
    int mapwin_h;
    int mapwin_y;
    int msg_w;
    int msg_h;
    int sb_w;
    int sb_x;
} terminal;

extern struct global g;
extern struct bitflags f;
extern struct terminal term;

#endif