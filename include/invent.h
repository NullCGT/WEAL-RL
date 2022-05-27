#ifndef INVENT_H
#define INVENT_H

#define MAX_INVENT_SIZE 26

struct item {
    struct actor *parent;
    int quan;               /* Quantity */
    int letter;       /* Previous letter used if dropped */
};

void init_item(struct actor *);
int display_invent(void);
int add_to_invent(struct actor *, struct actor *);

#endif