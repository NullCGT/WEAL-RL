#ifndef WINDOWS_H
#define WINDOWS_H

#include <curses.h>

/* Function Prototypes */
void title_screen(void);
void setup_gui(void);
WINDOW* create_win(int, int, int, int);
void cleanup_win(WINDOW *);
void setup_screen(void);
void cleanup_screen(void);
void text_entry(const char *, char *, int);
void display_file_text(const char *);
void display_sb(void);
int fullscreen_action(void);
void draw_msg_window(int);
int map_put_tile(int, int, int, int, int);
int map_put_actor(int, int, struct actor *, int);
int map_putch(int, int, int, int);
int map_putch_truecolor(int, int, int, unsigned);
void clear_map(void);
void refresh_map(void);
int handle_keys(void);
struct actor *win_pick_invent(void);

#define HUD_MODE_CHAR 0
#define HUD_MODE_NEAR 1
#define HUD_MODE_CONTROLS 2
#define MAX_HUD_MODE 3

#endif