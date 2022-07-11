#ifndef WINDOWS_H
#define WINDOWS_H

#include <curses.h>

/* Function Prototypes */
void title_screen(void);
WINDOW* create_win(int, int, int, int);
void cleanup_win(WINDOW *);
void setup_screen(void);
void cleanup_screen(void);
void text_entry(const char *, char *, int);
void display_file_text(const char *);
void display_energy_win(void);
void draw_msg_window(int, int);
int map_put_tile(int, int, int, int, int);
int map_put_actor(int, int, struct actor *, int);
int map_putch(int, int, int, int);
int map_putch_truecolor(int, int, int, unsigned);
void clear_map(void);
void refresh_map(void);
int handle_keys(void);
struct actor *win_pick_invent(void);

#endif