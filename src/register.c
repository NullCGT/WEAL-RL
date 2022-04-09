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
    .saved_locale = NULL
};

struct bitflags f = {
    .update_msg = 1,
    .update_map = 1,
    .update_fov = 1,
    .mode_explore = 0
};

struct terminal term = {
    .h = 36,
    .w = 90
};

struct monstat monstats[] = {
    {
        .id = 1,
        .name = "ketchucorgi",
        .description = "This adorable dog has bright red fur, a rotund body, and stubby legs. Upon seeing you, it barks happily.",
        .join = "The ketchucorgi barks happily. It's totally impressed by you!",
        .strength = 10,
        .speed = 10,
        .smart = 1,
        .tough = 5,
        .defense = 1,
        .evolve_to = 2
    }
};