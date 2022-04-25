#include <stddef.h>

#include "register.h"

struct global g = {
    .levmap = {{ {0} }},
    .player = NULL,
    .msg_list = NULL,
    .msg_last = NULL,
    .turns = 0,
    .depth = 1,
    .cx = 0,
    .cy = 0,
    .display_heat = 0,
    .prev_action = 0,
    .saved_locale = NULL
};

struct bitflags f = {
    .update_msg = 1,
    .update_map = 1,
    .update_fov = 1,
    .mode_explore = 0,
    .mode_run = 0,
    .mode_map = 1,
};

struct terminal term = {
    .h = 40,
    .w = 90
};

void setup_term_dimensions(int h, int w, int height_mul, int width_mul) {
    term.h = h;
    term.w = w;
    term.mapwin_w = term.w * 2 / 3 / width_mul;
    term.mapwin_h = min(MAPH, term.h * 5 / 6 / height_mul);
    term.msg_w = term.mapwin_w * width_mul - 2;
    term.msg_h = term.h - term.mapwin_h * height_mul;
    term.mapwin_y = term.msg_h / height_mul;
    term.sb_w = term.w - (term.mapwin_w * width_mul);
    term.sb_x = term.w - term.sb_w;
}