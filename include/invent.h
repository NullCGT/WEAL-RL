#ifndef INVENT_H
#define INVENT_H

#define MAX_INVENT_SIZE 26

struct item {
    struct actor *parent;
    int quan;               /* Quantity */
    unsigned char letter;       /* Previous letter used if dropped */
};

void init_item(struct actor *);

#endif