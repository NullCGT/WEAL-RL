/**
 * @file tile.c
 * @author Kestrel (kestrelg@kestrelscry.com)
 * @brief Functionality related to map tiles.
 * @version 1.0
 * @date 2022-05-27
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "tile.h"
#include "register.h"
#include "map.h"
#include "message.h"
#include "windows.h"
#include "render.h"


struct permtile permtiles[] = {
    PERMTILES
};

int *get_playerh(int x, int y) {
    return &g.levmap[x][y].player_heat;
}

int *get_exploreh(int x, int y) {
    return &g.levmap[x][y].explore_heat;
}

int *get_goalh(int x, int y) {
    return &g.levmap[x][y].goal_heat;
}

/**
 * @brief Initialize the tile struct.
 * 
 * @param intile The tile to be initialized. Mutated by this function.
 * @param tindex The index of the permtile to initialize the tile as.
 * @return A pointer to the modified tile.
 */
struct tile *init_tile(struct tile *intile, int tindex) {
    intile->color = permtiles[tindex].color;
    intile->pt = &permtiles[tindex];
    intile->actor = NULL;
    intile->item_actor = NULL;
    intile->refresh = 1;
    return intile;
}

/**
 * @brief Open a door.
 * 
 * @param actor The actor performing the action.
 * @param x x coordinate of the door.
 * @param y y coordinate of the door.
 * @return int The cost in energy of opening the door.
 */
int open_door(struct actor *actor, int x, int y) {
    struct tile *intile = &g.levmap[x][y];
    int tindex = intile->pt->id;

    if (tindex != T_DOOR_CLOSED) {
        logm("There is nothing to open in that direction.");
        return 0;
    }

    init_tile(intile, T_DOOR_OPEN); // init tile handles the refresh mark.
    if (is_visible(x, y)) {
        f.update_fov = 1;
    }

    if (actor != g.player && is_visible(actor->x, actor->y)) {
        logm("%s opens a door.", actor_name(actor, NAME_THE));
    } else if (is_visible(x, y))
        logm("The door opens.");
    return 100;
}

/**
 * @brief Close a door.
 * 
 * @param actor The actor performing the action.
 * @param x x coordinate of the door.
 * @param y y coordinate of the door.
 * @return int The cost in energy of opening the door.
 */
int close_door(struct actor *actor, int x, int y) {
    struct tile *intile = &g.levmap[x][y];
    int tindex = intile->pt->id;

    if (tindex != T_DOOR_OPEN) {
        logm("There is nothing to close in that direction.");
        return 0;
    }

    init_tile(intile, T_DOOR_CLOSED);
    if (is_visible(x, y)) {
        map_put_tile(x - g.cx, y - g.cy, x, y, intile->color);
        f.update_fov = 1;
        f.update_map = 1;
    }

    if (actor != g.player && is_visible(actor->x, actor->y)) {
        logm("%s closes a door.", actor_name(actor, NAME_THE));
    } else if (is_visible(x, y))
        logm("The door closes.");
    return 100;
}