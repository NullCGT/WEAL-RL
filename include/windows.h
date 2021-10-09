/* Screen-related functions */
WINDOW* setup_screen(void);
void cleanup_screen(void);
/* Window-related functions */
WINDOW* create_win(int, int, int, int);
void cleanup_win(WINDOW *);
/* Output-related functions */
int map_putch(int, int, int);