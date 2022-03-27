#include <stdlib.h>
#include "register.h"
#include "message.h"

void cast_light(int, int,
                double, double,
                int, int,
                int, int,
                int, int);
int make_visible(int, int);
void calculate_fov(int, int, int);

/* Cast light down an octant. Used for recursive shadowcasting. */
void cast_light(int cx, int cy,
                double start, double end,
                int row, int radius,
                int xx, int xy,
                int yx, int yy) {
    int rsq, blocked, dx, dy, x, y;
    double left_slope, right_slope, new_start;

    if (start < end || row > radius)
        return;
    
    rsq = radius * radius;

    for (int r = row; r <= radius; r++) {
        blocked = 0;
        dx = -1 * r - 1;
        dy = -1 * r;
        while (dx < 0) {
            dx++;
            /* Set up coordinates. If they are outside map
               bounds, continue. */
            x = cx + dx * xx + dy * xy;
            y = cy + dx * yx + dy * yy;
            if (x < 0 || x >= MAP_W || y < 0 || y >= MAP_H)
                continue;
            /* Set up slopes */
            left_slope = (dx - 0.5) / (dy + 0.5);
            right_slope = (dx + 0.5) / (dy - 0.5);
            /* Lighting calculations */
            if (start < right_slope)
                continue;
            else if (end > left_slope)
                break;
            else {
                if (dx * dx + dy * dy < rsq)
                    make_visible(x, y);
                if (blocked) {
                    if (g.levmap[x][y].blocked) {
                        new_start = right_slope;
                        continue;
                    } else {
                        blocked = 0;
                        start = new_start;
                    }
                } else if (g.levmap[x][y].blocked && r < radius) {
                    blocked = 1;
                    cast_light(cx, cy, start, left_slope,
                               r + 1, radius, xx, xy, yx, yy);
                    new_start = right_slope;
                }
            }
        }
        if (blocked)
            break;
    }
}

/* Make a point on the map visible. */
int make_visible(int x, int y) {
    g.levmap[x][y].visible = 1;
    g.levmap[x][y].explored = 1;
    if (g.levmap[x][y].opaque)
        return 1;
    return 0;
}

/* calculate the fov from a single point using recursive shadowcasting.

   Recursive shadowcasting is a 2001 FOV algorithm designed by
   Björn Bergström. It is described here:
   http://roguebasin.com/?title=FOV_using_recursive_shadowcasting
   
   This implementation is essentially a port of the python implementation
   by EricDB, described here:
   http://roguebasin.com/index.php/Python_shadowcasting_implementation */
void calculate_fov(int x, int y, int range) {
    /* Define octant multipliers. Due to the nature of static arrays in C,
       thses are initialized on the first call and remain in memory until
       the program exits. */
       static int m_0[] = {1,  0,  0, -1, -1,  0,  0,  1};
       static int m_1[] = {0,  1, -1,  0,  0, -1,  1,  0};
       static int m_2[] = {0,  1,  1,  0,  0, -1, -1,  0};
       static int m_3[] = {1,  0,  0,  1, -1,  0,  0, -1};
    /* Loop through each octant */
    for (int oct = 0; oct < 8; oct++) {
        cast_light(x, y, 1.0, 0.0, 1, range,
                    m_0[oct], m_1[oct], m_2[oct], m_3[oct]);
    }
}

/* Sets all tiles to not visible. */
void clear_fov(void) {
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            g.levmap[x][y].visible = 0;
        }
    }
}

/* Explores the entire map. */
void magic_mapping(void) {
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            g.levmap[x][y].explored = 1;
        }
    }
}