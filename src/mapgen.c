/**
 * @file mapgen.c
 * @author Kestrel (kestrelg@kestrelscry.com)
 * @brief Map generation functions. Currently undocumented due to high
 volatility.
 * @version 1.0
 * @date 2022-05-27
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <random.h>
#include <stdio.h>
#include "register.h"
#include "message.h"
#include "parser.h"
#include "map.h"
#include "spawn.h"

#define MIN_ROOM_W 3
#define MIN_ROOM_H 3

struct rect {
    unsigned char x1, y1;
    unsigned char x2, y2;
};

int in_region(struct rect, int, int);
struct rect create_room(struct rect);
void tunnel(struct rect, struct rect, int);
void do_bsp(struct rect, int, int, int, int);
void init_map(void);
void symmetrize(struct rect, struct rect, int);

/* Check if a set of coordinates fall inside of a given region. */
int in_region(struct rect region, int x, int y) {
    return (x >= region.x1 && x <= region.x2
            && y >= region.y1 && y <= region.y2);
}

/* Create a room, given a region. */
struct rect create_room(struct rect region) {
    int w, h, x0, y0;
    w = rndrng(MIN_ROOM_W, region.x2 - region.x1);
    h = rndrng(MIN_ROOM_H, region.y2 - region.y1);
    x0 = rndrng(region.x1, region.x2 - w);
    y0 = rndrng(region.y1, region.y2 - h);
    region.x1 = x0;
    region.x2 = x0 + w;
    region.y1 = y0;
    region.y2 = y0 + h;
    for (int x = region.x1; x <= region.x2; x++) {
        for (int y = region.y1; y <= region.y2; y++) {
            init_tile(&g.levmap[x][y], T_FLOOR);
        }
    }
    return region;
}

/* Punch a tunnel between two regions. */
void tunnel(struct rect region1, struct rect region2, int horiz) {
    int x, y, gx, gy;

    /* Randomly select source and destination coordinates. */
    do {
        x = rndrng(region1.x1 + 1, region1.x2 - 1);
        y = rndrng(region1.y1 + 1, region1.y2 - 1);
    } while (is_blocked(x, y));
    do {
        gx = rndrng(region2.x1 + 1, region2.x2 - 1);
        gy = rndrng(region2.y1 - 1, region2.y2 - 1);
    } while (is_blocked(gx, gy));

    while (x != gx || y != gy) {
        init_tile(&g.levmap[x][y], T_FLOOR);
        if (horiz) {
            if (x < gx) x++;
            else if (x > gx) x--;
            else if (y < gy) y++;
            else if (y > gy) y--;
        } else {
            if (y < gy) y++;
            else if (y > gy) y--;
            else if (x < gx) x++;
            else if (x > gx) x--;
        }
    }
}

/* 
region - The rectangle to perform BSP on.
min_w - The minimum width of a leaf.
min_h - The minimum heiight of a leaf.
horiz - Split vertically or horizontally.
depth - How many subdivisions to make. Decrements upon each recursion.
*/
void do_bsp(struct rect region, int min_w, int min_h, int horiz, int depth) {
    struct coord cut_loc;

    depth -= 1;

    /* If area is too small, carve out a room. */
    if ((horiz && (region.y2 - region.y1) <= 2 * min_h) 
        || (!horiz && (region.x2 - region.x1) <= 2 * min_w)) {
        horiz = !horiz;
    }
    if ((horiz && (region.y2 - region.y1) <= 2 * min_h) 
        || (!horiz && (region.x2 - region.x1) <= 2 * min_w)) {
        create_room(region);
        return;
    }
    
    /* Randomize cut location */
    if (!horiz) {
        cut_loc.x = rndrng(region.x1 + min_w, region.x2 - min_w);
        cut_loc.y = region.y1;
    } else {
        cut_loc.y = rndrng(region.y1 + min_h, region.y2 - min_h);
        cut_loc.x = region.x1;
    }

    /* make the cut */
    struct rect sub1 = { region.x1, region.y1, horiz ? region.x2 : cut_loc.x, horiz ? cut_loc.y : region.y2 };
    struct rect sub2 = { cut_loc.x, cut_loc.y, region.x2, region.y2 };

    /* Recurse, or create rooms if sufficient depth has been reached. */
    if (depth) {
        do_bsp(sub1, min_w, min_h, rndmx(2), depth);
        do_bsp(sub2, min_w, min_h, rndmx(2), depth);
    } else {
        sub1 = create_room(sub1);
        sub2 = create_room(sub2);
    }
    /* Connect subrooms when travelling back up the tree. */
    tunnel(sub1, sub2, horiz);
}

/* Initialize the map by making sure everything is not visible and
   not explored. */
void init_map(void) {
    for (int y = 0; y < MAPH; y++) {
        for (int x = 0; x < MAPW; x++) {
            init_tile(&g.levmap[x][y], T_WALL);
            g.levmap[x][y].lit = 0;
            g.levmap[x][y].visible = 0;
            g.levmap[x][y].explored = 0;
        }
    }
}

/* Mirrors region1 into region2. Assumes region1 and region2 are identical in size.*/
void symmetrize(struct rect region1, struct rect region2, int horiz) {
    int tindex;
    if (horiz) {
        for (int x = region1.x1; x < region1.x2; x++) {
            for (int y = region1.y1; y < region1.y2; y++) {
                tindex = g.levmap[x][y].pt->id;
                init_tile(&g.levmap[region2.x2 - x][y], tindex);
            }
        }
    } else {

    }
}

/* Create the level. */
void make_level(void) {
    struct coord c;
    /* Draw the map */
    struct rect level_rect = { 1, 1, MAPW / 2, MAPH - 1 };
    struct rect level_rect2 = { MAPW / 2, 1, MAPW - 1, MAPH - 1};
    init_map();
    do_bsp(level_rect, 8, 8, 1, 7);
    symmetrize(level_rect, level_rect2, 1);
    c = rand_open_coord();
    init_tile(&g.levmap[c.x][c.y], T_STAIR_UP);
    c = rand_open_coord();
    init_tile(&g.levmap[c.x][c.y], T_STAIR_DOWN);
    /* Populate level */
    for (int i = 0; i < 3; i++) {
        spawn_creature(M_ZOMBIE, -1, -1);
    }


    f.update_map = 1;
    f.update_fov = 1;
    return;
}