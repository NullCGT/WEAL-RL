#include <stddef.h>

#include "register.h"

struct global g = {
    .levmap = {{ {0} }},
    .player = { {0} },
    .msg_list = NULL,
    .turns = 0,
    .saved_locale = NULL,
    .map_win = NULL,
    .msg_win = NULL,
};

struct bitflags f = {
    .update_msg = 1,
    .update_map = 1,
    .update_sidebar = 1,
};