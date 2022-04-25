#ifndef INVENT_H
#define INVENT_H

#define MAX_INVENT_SIZE 26

struct invent {
    struct actor *parent;
    struct actor *items[MAX_INVENT_SIZE];
};

struct item {
    struct actor *parent;
    int quan;               /* Quantity */
    char prev_letter;       /* Previous letter used if dropped */
};

void init_invent(struct actor *);
void free_invent(struct invent *);
void init_item(struct actor *);

#endif