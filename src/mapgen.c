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
void cellular_automata(struct rect, int, int);
int deisolate(void);
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

/**
 * @brief Carve out a portion of the dungeon level using a cellular automata
 algorithm.
 * 
 * @param region The region to perform cellular automata upon.
 * @param filled An integer between 0 and 100. Used to determine what
 percentage of the map should start filled.
 * @param iterations How many iterations to run the algorithm.
 */
void cellular_automata(struct rect region, int filled, int iterations) {
    int x, y, nx, ny;
    int alive;
    int width = region.x2 - region.x1;
    int height = region.y2 - region.y1;
    unsigned char cells[width][height];

    /* Initialize cells. */
    for (x = 0; x < width; x++) {
        for (y = 0; y < height; y++) {
            cells[x][y] = (rndmx(100) < filled);
        }
    }
    /* Game of Life */
    for (int i = 0; i < iterations; i++) {
        for (x = 0; x < width; x++) {
            for (y = 0; y < height; y++) {

                alive = 0;
                for (int x1 = -1; x1 <= 1; x1++) {
                    nx = x + x1;
                    if (nx < 0 || nx >= width) {
                        alive += 3;
                        continue;
                    }
                    for (int y1 = -1; y1 <= 1; y1++) {
                        ny = y + y1;
                        if (ny < 0 || ny >= height) {
                            alive++;
                        } else if (cells[nx][ny]) {
                            alive++;
                        }
                    }
                }
                if (alive >= 5)
                    cells[x][y] = 1;
                else
                    cells[x][y] = 0;
            }
        }
    }

    /* Transfer to grid */
    for (x = 0; x < width; x++) {
        for (y = 0; y < height; y++) {
            if (!cells[x][y])
                init_tile(&g.levmap[region.x1 + x][region.y1 + y], T_FLOOR);
        }
    }
}

/**
 * @brief Combs the level map for isolated areas. Upon finding one, starts a
 random walk, which finishes when the level is fully connected.
 * 
 * @return int Return 1 if changes were made to the level, otherwise return 0.
 */
int deisolate(void) {
    int x, y;
    int dx, dy;
    /* Hacky hack */
    for (x = 0; x < MAPW; x++) {
        for (y = 0; y < MAPH; y++) {
            if (!is_blocked(x, y)) {
                break;
            }
        }
        if (!is_blocked(x, y)) {
            break;
        }
    }
    g.player->x = x;
    g.player->y = y;
    do_heatmaps();
    /* Loop over everything to see if there is somewhere the player cannot get. */
    for (x = 0; x < MAPW; x++) {
        for (y = 0; y < MAPH; y++) {
            if (g.levmap[x][y].player_heat == MAX_HEAT) {
                /* Hulk walk until we make a connection with an explorable area. */
                do {
                    dx = rndrng(-1, 2);
                    dy = rndrng(-1, 2);
                    if (x + dx < 1 || x + dx >= MAPW - 1)
                        dx = dx * -1;
                    if (y + dy < 1 || y + dy >= MAPH - 1)
                        dy = dy * -1;
                    x += dx;
                    y += dy;
                    init_tile(&g.levmap[x][y], T_FLOOR);
                } while (g.levmap[x][y].player_heat >= MAX_HEAT);
                return 1;
            }
        }
    }
    return 0;
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
    struct rect level_rect = { 1, 1, MAPW - 1, MAPH - 1 };
    /* Temporary: Parse the dungeon. */
    dungeon_from_file("data/dungeon/limbo.json");
    /* Draw the map */
    init_map();
    cellular_automata(level_rect, 45, 5);
    while (deisolate()) {
        //logm("Connecting an isolated area.");
    };
    
    c = rand_open_coord();
    init_tile(&g.levmap[c.x][c.y], T_STAIR_UP);
    c = rand_open_coord();
    init_tile(&g.levmap[c.x][c.y], T_STAIR_DOWN);
    /* Populate level */
    for (int i = 0; i < 3; i++) {
        c = rand_open_coord();
        spawns_from_dungeon(dgn.filename, g.depth, c.x, c.y);
    }
    for (int i = 0; i < 10; i++) {
        spawn_item("knife", -1, -1);
    }

    f.update_map = 1;
    f.update_fov = 1;
    return;
}