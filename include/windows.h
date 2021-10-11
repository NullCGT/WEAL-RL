/* Screen-related functions */
void setup_screen(void);
void cleanup_screen(void);
/* Window-related functions */
WINDOW* create_win(int, int, int, int);
void cleanup_win(WINDOW *);
/* Output-related functions */
int map_putch(int, int, int);

/* COLOR DEFINITIONS */
#define BLACK 0
#define RED 1
#define GREEN 2
#define YELLOW 3
#define BLUE 4
#define MAGENTA 5
#define CYAN 6
#define WHITE 7