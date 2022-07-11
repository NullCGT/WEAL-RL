#ifndef PARSER_H
#define PARSER_H

/* Function Prototypes */
struct actor *actor_from_file(const char *);
void dungeon_from_file(const char *);
void spawns_from_dungeon(const char *, int, int, int);

#endif