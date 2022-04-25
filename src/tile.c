#include "tile.h"


struct permtile permtiles[] = {
    PERMTILES
};

void init_tile(struct tile *intile, int tindex) {
    intile->color = permtiles[tindex].color;
    intile->pt = &permtiles[tindex];
    intile->actor = NULL;
    intile->item_actor = NULL;
}