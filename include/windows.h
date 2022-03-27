#ifndef WINDOWS_H
#define WINDOWS_H

void setup_screen(void);
void cleanup_screen(void);
WINDOW* create_win(int, int, int, int);
void cleanup_win(WINDOW *);
void create_popup_win(const char *, const char *);
void display_energy_win(void);
void draw_msg_window(WINDOW *, int);
void full_msg_window(void);
int map_putch(int, int, int, int);
void refresh_map(void);

#endif