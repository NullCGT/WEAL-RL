#ifndef ACTION_H
#define ACTION_H

#define DEBUG_CUTOFF 100

int move_mon(struct npc*, int, int);
void execute_action(int);

enum actionnum {
    A_NONE,
    A_WEST,
    A_EAST,
    A_NORTH,
    A_SOUTH,
    A_NORTHWEST,
    A_NORTHEAST,
    A_SOUTHWEST,
    A_SOUTHEAST,
    A_REST,
    A_ASCEND,
    A_DESCEND,
    A_FULLSCREEN,
    A_QUIT,
    A_DEBUG_MAGICMAP = DEBUG_CUTOFF
};


#endif