#include <stdlib.h>
#include <time.h>

/* Seed the random number generator with a given value. */
void rndseed(int x) {
    srand(x);
    return;
}

/* Seed the random number generator with a random value. */
void rndseed_t() {
    rndseed((unsigned) time(NULL));
    return;
}

/* Return a random number greater than or equal to zero and less than x. */
int rndmx(int x) {
    return (rand() % x);
}

/* Return a random number greater than or equal to x and less than y. */
int rndrng(int x, int y) {
    if (y <= x) return x;
    return (x + (rand() % (y - x)));
}

/* Return a random boolean value. */
int rndbool() {
    return (rand() % 2);
}

/* Roll xdy and return the result. */
int d(int x, int y) {
    int ret = 0;
    for (int i = 0; i < x; i++) {
        ret += rndrng(1, 7);
    }
    return ret;
}