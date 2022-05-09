#include "tile.h"
#include "register.h"
#include "map.h"
#include "message.h"
#include "windows.h"


struct permtile permtiles[] = {
    PERMTILES
};

void init_tile(struct tile *intile, int tindex) {
    intile->color = permtiles[tindex].color;
    intile->pt = &permtiles[tindex];
    intile->actor = NULL;
    intile->item_actor = NULL;
}

int open_door(struct actor *actor, int x, int y) {
    struct tile *intile = &g.levmap[x][y];
    int tindex = intile->pt->id;

    if (tindex != T_DOOR_CLOSED) {
        logm("There is no door to open here.");
        return 0;
    }

    init_tile(intile, T_DOOR_OPEN);
    if (is_visible(x, y)) {
        map_put_tile(x - g.cx, y - g.cy, x, y, intile->color);
        f.update_fov = 1;
        f.update_map = 1;
    }

    if (actor != g.player && is_visible(actor->x, actor->y)) {
        logm("%s opens a door.", actor_name(actor, NAME_THE));
    } else if (is_visible(x, y))
        logm("The door opens.");
    return 100;
}