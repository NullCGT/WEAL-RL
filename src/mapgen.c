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

#include <stdlib.h>

#include <random.h>
#include <stdio.h>
#include "register.h"
#include "message.h"
#include "parser.h"
#include "map.h"
#include "spawn.h"

struct rect {
    unsigned char x1, y1;
    unsigned char x2, y2;
};

struct bsp_node {
    struct rect region;
    struct bsp_node *left;
    struct bsp_node *right;
};

struct bsp_node *create_bsp_node(struct rect);
void dig_rooms(struct bsp_node *);
void connect_regions(struct bsp_node *);
void tunnel(struct coord c1, struct coord c2);
void free_bsp(struct bsp_node *);

struct rect create_room(struct rect);
void cellular_automata(struct rect, int, int);
int deisolate(void);
struct bsp_node *bsp(struct rect, int, int);
void beautify_map(void);
void init_map(void);

/* Carve out a room, given a region. */
struct rect create_room(struct rect region) {
    int w, h, x0, y0;
    w = region.x2 - region.x1;
    h = region.y2 - region.y1;
    x0 = region.x1;
    y0 = region.y1;
    for (int x = x0; x <= x0 + w; x++) {
        for (int y = y0; y <= y0 + h; y++) {
            //if (g.levmap[x][y].pt->id != T_EARTH)
            //    continue;
            if (x == x0 || x == x0 + w || y == y0 || y == y0 + h) {
                init_tile(&g.levmap[x][y], T_WALL);
            } else {
                init_tile(&g.levmap[x][y], T_FLOOR);
            }
        }
    }
    return region;
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
    int blocked = 1;
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
            if (!cells[x][y]) {
                init_tile(&g.levmap[region.x1 + x][region.y1 + y], T_FLOOR);
                blocked = 0;
            }
        }
    }
    /* Add a single cell if none existed after running (can happen on small inputs) */
    if (blocked) {
        init_tile(&g.levmap[rndrng(region.x1, region.x1 + x)][rndrng(region.y1, region.y1 + 1)], T_FLOOR);
    }
}

/**
 * @brief Combs the level map for isolated areas. Upon finding one, use a
 dijkstra map in order to connect it. Highly expensive.
 * 
 * @return int Return 1 if changes were made to the level, otherwise return 0.
 */
int deisolate(void) {
    int x, y;
    int dx, dy;
    struct coord c1, c2;
    /* Hacky hack */
    for (dx = 0; dx < MAPW; dx++) {
        for (dy = 0; dy < MAPH; dy++) {
            if (!is_blocked(dx, dy)) {
                break;
            }
        }
        if (!is_blocked(dx, dy)) {
            break;
        }
    }
    generic_heatmap(dx, dy, 0);
    /* Loop over everything to see if there is somewhere the player cannot get. */
    for (x = 0; x < MAPW; x++) {
        for (y = 0; y < MAPH; y++) {
            if (g.levmap[x][y].generic_heat == MAX_HEAT) {
                c1.x = dx;
                c1.y = dy;
                c2.x = x;
                c2.y = y;
                generic_heatmap(x, y, 1);
                tunnel(c1, c2);
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
            init_tile(&g.levmap[x][y], T_EARTH);
            g.levmap[x][y].lit = 0;
            g.levmap[x][y].visible = 0;
            g.levmap[x][y].explored = 0;
        }
    }
}

struct bsp_node *create_bsp_node(struct rect region) {
    struct bsp_node *node = (struct bsp_node *) malloc(sizeof(struct bsp_node));
    node->region = region;
    node->left = NULL;
    node->right = NULL;
    return node;
}

void free_bsp(struct bsp_node *node) {
    if (!node) return;
    free_bsp(node->left);
    free_bsp(node->right);
    free(node);
}

void dig_rooms(struct bsp_node *node) {
    if (!node) return;
    dig_rooms(node->left);
    dig_rooms(node->right);
    if (node->right == NULL && node->left == NULL) {
        #if 0
        create_room(node->region);
        #else
        if (rndmx(2))
            cellular_automata(node->region, 55, 1);
        else
            create_room(node->region);
        #endif
    }
}

void connect_regions(struct bsp_node *node) {
    struct coord c1, c2;
    if (!node || (node->left == NULL && node->right == NULL)) return;

    connect_regions(node->left);
    connect_regions(node->right);
    do {
        c1.x = rndrng(node->left->region.x1, node->left->region.x2);
        c1.y = rndrng(node->left->region.y1, node->left->region.y2);
    } while (is_blocked(c1.x, c1.y));
    do {
        c2.x = rndrng(node->right->region.x1, node->right->region.x2);
        c2.y = rndrng(node->right->region.y1, node->right->region.y2);
    } while (is_blocked(c2.x, c2.y));

    generic_heatmap(c2.x, c2.y, 1);
    tunnel(c1, c2);
}

void tunnel(struct coord c1, struct coord c2) {
    struct coord nc;
    while (c1.x != c2.x || c1.y != c2.y) {
        nc = best_adjacent_tile(c1.x, c1.y, 0, get_generich);
        c1.x += nc.x;
        c1.y += nc.y;
        if (is_blocked(c1.x, c1.y))
            init_tile(&g.levmap[c1.x][c1.y], T_FLOOR);
        if (c1.x == 0 && c1.y == 0) return;
    }
}

void beautify_map(void) {
    int blocked, door_candidate;
    int walls, floors;
    for (int x = 0; x < MAPW; x++) {
        for (int y = 0; y < MAPH; y++) {
            /* Loop over neighbors */
            blocked = is_wall(x, y);
            door_candidate = 0;
            walls = 0;
            floors = 0;
            for (int dx = -1; dx <= 1; dx++) {
                for (int dy = -1; dy <= 1; dy++) {
                    if (!in_bounds(dx + x, dy + y)) {
                        walls++;
                        continue;
                    }
                    if (is_wall(dx + x, dy + y)) { 
                        door_candidate = 
                            (door_candidate || 
                                (!blocked && (!dx || !dy) && in_bounds(x - dx, y - dy) && is_wall(x - dx, y - dy)));
                        walls++;
                    }
                    else floors++;
                }
            }
            if (blocked && floors) init_tile(&g.levmap[x][y], T_WALL);
            if (door_candidate && walls > 1 && walls < 6) init_tile(&g.levmap[x][y], T_DOOR_CLOSED);
        }
    }
}

struct bsp_node *bsp(struct rect region, int min_radius, int depth) {
    int w = region.x2 - region.x1;
    int h = region.y2 - region.y1;
    int horiz;
    struct rect r1;
    struct rect r2;
    struct bsp_node *node;

    /* Create the bsp node */
    node = create_bsp_node(region);
    
    /* Decide which way to make the cut. */
    if (!depth || (w <= min_radius * 2 && h <= min_radius * 2)) {
        //create_room(region);
        return node;
    } else if (w <= min_radius * 2) {
        horiz = 1;
    } else if (h <= min_radius * 2) {
        horiz = 0;
    } else {
        horiz = rndmx(2);
    }

    if (horiz) {
        r1.x1 = region.x1;
        r1.x2 = region.x2;
        r1.y1 = region.y1;
        r1.y2 = rndrng(region.y1 + min_radius, region.y2 - min_radius);

        r2.x1 = region.x1;
        r2.x2 = region.x2;
        r2.y1 = r1.y2;
        r2.y2 = region.y2;
    } else {
        r1.y1 = region.y1;
        r1.y2 = region.y2;
        r1.x1 = region.x1;
        r1.x2 = rndrng(region.x1 + min_radius, region.x2 - min_radius);

        r2.y1 = region.y1;
        r2.y2 = region.y2;
        r2.x1 = r1.x2;
        r2.x2 = region.x2;
    }

    depth--;
    node->left = bsp(r1, min_radius, depth);
    node->right = bsp(r2, min_radius, depth);
    return node;
}

void make_level(void) {
    struct bsp_node *bsp_tree;
    struct rect central_rect = { 0, 0, MAPW - 1, MAPH - 1 };
    /* Temporary: Parse the dungeon. */
    dungeon_from_file("data/dungeon/limbo.json");
    init_map();
    bsp_tree = bsp(central_rect, 6, 6);
    dig_rooms(bsp_tree);
    connect_regions(bsp_tree);
    free_bsp(bsp_tree);
    while(deisolate());
    beautify_map();

    #if 0
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
    #endif

    f.update_map = 1;
    f.update_fov = 1;
    return;
}