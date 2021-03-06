#ifndef CREATURE_H
#define CREATURE_H

/* Function Prototypes */
struct name *init_permname(struct actor *, const char *);
struct actor *add_actor_to_main(struct actor *);
struct actor *spawn_creature(const char *name, int x, int y);
struct actor *spawn_item(const char *name, int x, int y);
int debug_summon(void);
int debug_wish(void);

#endif