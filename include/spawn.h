#ifndef CREATURE_H
#define CREATURE_H

/* Function Prototypes */
struct name *init_permname(struct actor *, const char *);
struct actor *spawn_creature(const char *name, int x, int y);
struct actor *spawn_item(const char *name, int x, int y);

#endif