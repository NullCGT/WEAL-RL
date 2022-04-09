#ifndef WINDOWS_H
#define WINDOWS_H

void setup_screen(void);
void cleanup_screen(void);
void create_popup_win(const char *, const char *);
void display_energy_win(void);
void draw_msg_window(int, int);
int map_putch(int, int, int, int);
int map_putch_truecolor(int, int, int, unsigned);
void clear_map(void);
void refresh_map(void);
int handle_keys(void);

#endif