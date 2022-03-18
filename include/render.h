#ifndef RENDER_H
#define RENDER_H

void render_map(void);
void render_all_npcs(void);
void clear_npcs(void);
int map_putch(int, int, int);
void render_bar(WINDOW*, int, int, int, int, int, char, char);

#endif