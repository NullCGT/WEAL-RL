#ifndef FUNCTIONS_H
#define FUNCTIONS_H

/* MAPGEN.H */
void make_level(void);

/* MESSAGE.H */
void free_message_list(struct msg *);
void full_msg_window(void);
int draw_msg_window(WINDOW *);
int logm(const char *, ...);
int logma(int, const char *, ...);

/* MONSTER.H */
struct monster *create_monster(int, int);
void do_wild_encounter(void);

/* RENDER.H */
void render_map(void);
void render_all_npcs(void);
void clear_npcs(void);
int map_putch(int, int, int);

/* WINDOWS.H */
void setup_screen(void);
void cleanup_screen(void);
WINDOW* create_win(int, int, int, int);
void cleanup_win(WINDOW *);
void create_popup_win(const char *, const char *);

#endif