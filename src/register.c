#include <stddef.h>

#include "register.h"

struct global g = {
    .levmap = {{ {0} }},
    .player = { {0} },
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
    .h = 36,
    .w = 90
};