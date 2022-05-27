/**
 * @file register.c
 * @author your name (you@domain.com)
 * @brief Holds information about the overall game state.
 * @version 1.0
 * @date 2022-05-27
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <stddef.h>

#include "register.h"

struct global g = {
    .levmap = {{ {0} }},
    .player = NULL,
    .killer = NULL,
    .msg_list = NULL,
    .msg_last = NULL,
    .turns = 0,
    .depth = 1,
    .cx = 0,
    .cy = 0,
    .cursor_x = 0,
    .cursor_y = 0,
    .goal_x = -1,
    .goal_y = -1,
    .display_heat = 0,
    .prev_action = 0,
};

struct bitflags f = {
    .update_msg = 1,
    .update_map = 1,
    .update_fov = 1,
    .mode_explore = 0,
    .mode_run = 0,
    .mode_map = 1,
    .mode_look = 0,
};

struct terminal term = {
    .h = 40,
    .w = 90
};

/**
 * @brief Sets up the terminal dimensions.
 * 
 * @param h Height.
 * @param w Width.
 * @param height_mul Height multiplier if using scaled tiles.
 * @param width_mul Width multiplier if using scaled tiles.
 */
void setup_term_dimensions(int h, int w, int height_mul, int width_mul) {
    term.h = h;
    term.w = w;
    term.mapwin_w = term.w * 2 / 3 / width_mul;
    term.mapwin_h = min(MAPH, term.h * 5 / 6 / height_mul);
    term.msg_w = term.mapwin_w * width_mul;
    term.msg_h = term.h - term.mapwin_h * height_mul;
    term.mapwin_y = term.msg_h / height_mul;
    term.sb_w = term.w - (term.mapwin_w * width_mul);
    term.sb_x = term.w - term.sb_w;
}