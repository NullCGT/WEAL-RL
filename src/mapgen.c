#define WFC_IMPLEMENTATION

#include <random.h>
#include <stdio.h>
#include "register.h"
#include "message.h"
#include "parser.h"

void init_map(void);
int wfc_magpen(void);

#define WFC_SUCCESS 0
#define WFC_ERROR 1
#define WFC_TRIES 10

int wfc_mapgen(void) {
    struct wfc_image image = parse_wfc_xml("data/wfc/corp0.xml");

    /* NOTE: wfc_overlapping includes a call to wfc_init, which calls srand(),
       destroying the current rng seed. When setseed mode is implemented, we
       will need to work around this, or modify wfc.h. */
    /* Additionally, note that this function massively leaks memory. I've opened
       an issue on github with the author, so hopefully this will be resolved in
       the future. */
    struct wfc *wfc = wfc_overlapping(MAPW,
                                    MAPH,
                                    &image,
                                    3,
                                    3,
                                    1,
                                    1,
                                    1,
                                    1); 

    if (wfc == NULL) {
        logm("Error: cannot create wfc.");
        wfc_img_destroy(&image);
        wfc_destroy(wfc);
        return WFC_ERROR;
    }

    if (!wfc_run(wfc, -1)) {
        logm("Error: Something went wrong with wfc.");
        wfc_img_destroy(&image);
        wfc_destroy(wfc);
        return WFC_ERROR;
    }
    struct wfc_image *output_image = wfc_output_image(wfc);
    if (!output_image) {
        logm("Error: FAILURE.");
        wfc_img_destroy(&image);
        wfc_destroy(wfc);
        return WFC_ERROR;
    }
    for (int y = 0; y < MAPH; y++) {
        for (int x = 0; x < MAPW; x++) {
            unsigned char cell = output_image->data[y * MAPW + x];
            if (cell == '.') {
                init_tile(&g.levmap[x][y], T_FLOOR);
            } else {
                init_tile(&g.levmap[x][y], T_WALL);
            }
        }
    }
    /* Clean leftover data from xml parser. */
    free(image.data);
    /* Clean other memory. */
    wfc_img_destroy(output_image);
    wfc_destroy(wfc);
    return WFC_SUCCESS;
}

/* Initialize the map by making sure everything is not visible and
   not explored. */
void init_map(void) {
    for (int y = 0; y < MAPH; y++) {
        for (int x = 0; x < MAPW; x++) {
            g.levmap[x][y].lit = 0;
            g.levmap[x][y].visible = 0;
            g.levmap[x][y].explored = 0;
        }
    }
}

/* Create the level */
void make_level(void) {
    int success = 0;

    init_map();
    for (int tries = 0; tries < WFC_TRIES; tries++) {
        if (!wfc_mapgen()) {
            success = !success;
            break;
        }
    }
    /* Handle map creation failure. */
    if (!success) {
        logm("Map creation failed: WFC attempts >= %d.", WFC_TRIES);
    }
    f.update_map = 1;
    f.update_fov = 1;
    return;
}