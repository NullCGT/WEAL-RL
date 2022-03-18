#ifndef REGISTER_H
#define REGISTER_H

#include <curses.h>
#include "color.h"

#define MAP_W 80
#define MSG_W MAP_W
#define SB_W 30

#define MAP_H 30
#define MSG_H 6
#define SB_H MAP_H + MSG_H

#define MAP_Y MSG_H
#define MSG_Y 0
#define SB_Y 0

#define SB_X MAP_W

struct action {
    char *name;
    char *desc;
    void *func;
    struct action *next;
};

struct npc {
    char name[20];
    int chr;
    int x;
    int y;
    int energy;
    int emax;
    struct action *actions;
    struct npc *next;
    /* bitfields */
    unsigned int playable : 1;
    /* 7 free bits */
};

struct monstat {
    int id;
    const char *name;
    const char *description;
    const char *join;
    int strength;
    int speed;
    int smart;
    int tough;
    int defense;
    int evolve_to;
};

struct monster {
    int level;
    int cur_hp;
    int max_hp;
    struct monstat *monstat;
};

struct tile {
    int chr;
    int blocked;
};

typedef struct global {
    struct tile levmap[MAP_W][MAP_H];
    struct npc player; /* Assume player is first NPC */
    struct msg *msg_list;
    int turns;
    char *saved_locale;
    WINDOW *map_win;
    WINDOW *msg_win;
} global;

typedef struct bitflags {
    unsigned int update_msg : 1;
    unsigned int update_map : 1;
    /* 6 free bits */
} bitflags;

extern struct global g;
extern struct bitflags f;
extern struct monstat monstats[];

#endif