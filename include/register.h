#ifndef REGISTER_H
#define REGISTER_H

#include <curses.h>
#include "color.h"

#define MAP_WIDTH 80
#define MAP_HEIGHT 30

#define SB_W 20
#define SB_H 32

#define MSG_W MAP_WIDTH + 2
#define MSG_H 6
#define MSG_Y MAP_HEIGHT + 2

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
    struct action *actions;
    struct npc *next;
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
    struct tile levmap[MAP_WIDTH][MAP_HEIGHT];
    struct npc player;
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