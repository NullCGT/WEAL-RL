#ifndef COLOR_H
#define COLOR_H

struct w_color {
    const char *str;
    unsigned char cnum;
    unsigned char r, g, b;
};

enum colornum {
    BLACK,
    RED,
    GREEN,
    YELLOW,
    BLUE,
    MAGENTA,
    CYAN,
    WHITE
};

#define MAX_COLOR WHITE + 1

extern struct w_color w_colors[MAX_COLOR];

#endif