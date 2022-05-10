#ifndef ACTION_H
#define ACTION_H

#define DEBUG_CUTOFF 100

int move_mon(struct actor*, int, int);
int get_action(void);
int dir_to_action(int, int);
int execute_action(struct actor*, int);

/* Order is important! */
enum actionnum {
    A_NONE,
    /* Movement actions */
    A_WEST,
    A_EAST,
    A_NORTH,
    A_SOUTH,
    A_NORTHWEST,
    A_NORTHEAST,
    A_SOUTHWEST,
    A_SOUTHEAST,
    /* End movement actions */
    A_REST,
    A_OPEN,
    A_CLOSE,
    A_ASCEND,
    A_DESCEND,
    A_PICK_UP,
    A_LOOK_DOWN,
    A_EXPLORE,
    A_FULLSCREEN,
    A_HELP,
    A_SAVE,
    A_QUIT,
    A_DEBUG_MAGICMAP = DEBUG_CUTOFF,
    A_DEBUG_HEAT
};

#define is_movement(a) \
    (a > A_NONE && a < A_REST)


#endif