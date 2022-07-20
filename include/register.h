#ifndef REGISTER_H
#define REGISTER_H

#include "actor.h"
#include "color.h"
#include "tile.h"

/* Map and window constants */
#define MAPW 80
#define MAPH 40
#define MIN_TERM_H 20
#define MIN_TERM_W 60

/* Common functions */
#define max(x, y) (((x) > (y)) ? (x) : (y))
#define min(x, y) (((x) < (y)) ? (x) : (y))
#define signum(x) ((x > 0) - (x < 0))
#define vowel(x) (x == 'a' || x == 'e' || x == 'i' || x == 'o' || x == 'u')

/* Func Proto */
void setup_term_dimensions(int, int, int, int);

typedef struct global {
    struct tile levmap[MAPW][MAPH];
    struct actor *player; /* Assume player is first NPC */
    struct actor *target;
    struct actor *active_attacker;
    struct msg *msg_list;
    struct msg *msg_last;
    struct action *prev_action; /* for the moment, only used for runmode */
    unsigned char active_attack_index;
    int turns;
    int depth;
    int max_depth;
    int score;
    int cx, cy; /* Camera location */
    int cursor_x, cursor_y; /* In-game cursor location */
    int goal_x, goal_y; /* Traveling */
    int display_heat;
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
    unsigned int mode_look : 1;
    /* 1 free bit */
} bitflags;

typedef struct dungeon {
    char filename[64];
    char name[64];
    unsigned char randomness;       /* Chance of disregarding encounter list to spawn a random creature. */
    unsigned char wall_color;       /* Color of the dungeon walls. */
    unsigned long forbidden_tags;   /* Actors with this tag cannot be randomly generated here. */
    unsigned long preferred_tags;   /* Actors with this tag preferrentially generate here. */
    unsigned long required_tags;   /* Actors that spawn randomly must have one of these tags. */
} dungeon;

typedef struct terminal {
    int h;
    int w;
    int mapwin_w;
    int mapwin_h;
    int mapwin_y;
    int msg_w;
    int msg_h;
    int msg_y;
    int sb_w;
    int sb_x;
    int sb_h;
    char *saved_locale;
    char hudmode;
} terminal;

extern struct global g;
extern struct bitflags f;
extern struct dungeon dgn;
extern struct terminal term;

#endif