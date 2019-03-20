/* See LICENSE for license details. */
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <time.h>

static int m[4][4] = { 0 }; /* game board */

/* colors for tiles (for ANSI escape sequences \033[38;5;x )*/
static uint8_t bgcolor = 0;
static uint8_t fgcolor[] = {
    [0] = 0, /* same as bgcolor, so no not displayed */
    [2] = 1,
    [4] = 2,
    [8] = 3,
    [16] = 4,
    [32] = 5,
    [64] = 6,
    [128] = 7,
    [256] = 8,
    [512] = 9,
    [1024] = 10,
    [2048] = 11,
};

static void display()
{
    int i, j;
    for (i = 0; i < 4; ++i) {
        for (j = 0; j < 4; ++j) {
            printf("\033[48;5;%dm\033[38;5;%dm%5d\033[0m",
                    bgcolor, fgcolor[m[i][j]], m[i][j]);
            if (j < 4)
                putchar(' ');
        }
        putchar('\n');
        putchar('\n');
    }
}

static int nmoves = 0; /* number of tiles shifted/added in previous move */
static int won = 0; /* game won? */
static int score = 0; /* game score */

static void tile_swap(int *a, int *b) {
    /* swap in direction b --> a */
    if (!*a && *b) {
        *a = *b;
        *b = 0;

        nmoves++;
    }
}

static void tile_sum(int *a, int *b) {
    /* sum in direction b --> a */
    if (*a && *a == *b) {
        *a = *a + *b;
        *b = 0;

        if (*a == 2048)
            won = 1;

        nmoves++;
        score += *a;
    }
}

#define INT(b) ((b)? 1: 0)
#define NEG(b) ((b)? -1: 1)

static void pair_iterator(int dim, int lr, void (*fn)(int*, int*))
{
    int i[2], ii[2]; /* 2d loop variables */
    int r_offset[2] = { /* index offset for next tile in shift direction */
        NEG(lr) * INT(dim == 0),
        NEG(lr) * INT(dim == 1)
    };
    int odim = (dim + 1) % 2;
    int *left, *right;
    
    for (i[dim] = 0; i[dim] < 3; ++i[dim]) {
        for (i[odim] = 0; i[odim] < 4; ++i[odim]) {
            /* transpose iteration on right/down */
            ii[0] = lr? 3 - i[0]: i[0];
            ii[1] = lr? 3 - i[1]: i[1];

            left = &m[ii[0]][ii[1]];
            right = &m[ii[0] + r_offset[0]][ii[1] + r_offset[1]];

            fn(left, right);
        }
    }
}

/* shift (swap and add) tiles
   dim == 0: x-direction; dim == 1: y-direction
   lr == 0: left/up; lr == 1: right/down (depending on "dim") */
static void shift(int dim, int lr)
{
    /* 3 rounds of shifting required, sum tiles after second
       ensures full shift of all tiles, all possible sums and
       full shift of sums. */
    pair_iterator(dim, lr, tile_swap);
    pair_iterator(dim, lr, tile_swap);
    pair_iterator(dim, lr, tile_sum);
    pair_iterator(dim, lr, tile_swap);
}

/* number of empty tiles */
static int n_empty_tiles()
{
    int i, j, n = 0;
    for (i = 0; i < 4; ++i)
        for (j = 0; j < 4; ++j)
            if (!m[i][j])
                n++;
    return n;
}

static int is_sum_possible = 0; /* state variable of sum_possible() */

static void sum_possible_impl(int *a, int *b)
{
    if (*a && *a == *b)
       is_sum_possible = 1;
}

static int sum_possible()
{
    is_sum_possible = 0;
    /* need only to examine direct neighbors
       as this is only relevant on full boards */
    pair_iterator(0, 0, sum_possible_impl);
    pair_iterator(1, 0, sum_possible_impl);
    return is_sum_possible;
}

static int move_possible()
{
    return n_empty_tiles() != 0 || sum_possible();
}

/* uniform random number in [0, ub) */
static int uniform_random(int ub)
{
    int lim = RAND_MAX - RAND_MAX % ub;
    int r;

    do {
        r = rand();
    } while (r >= lim);
    return r % ub;
}

/* randomly add a new empty tile */
static void add_tiles()
{
    int i, j;
    int r = uniform_random(n_empty_tiles());

    for (i = 0; i < 4; ++i)
        for (j = 0; j < 4; ++j)
            if (!m[i][j] && r-- == 0)
                m[i][j] = 2;
}


static struct termios tio; /* termios state */

static void reset_termios()
{
    tcsetattr(0, TCSANOW, &tio);
}

static void shandler(int signal)
{
    (void) signal;
    reset_termios();
    exit(2);
}

int main()
{
    int c, ret = 0;
    struct termios ntio;

    /* disable line-based input and echo */
    tcgetattr(0, &tio);
    ntio = tio;
    ntio.c_lflag &= ~ICANON & ~ECHO;
    tcsetattr(0, TCSANOW, &ntio);
    signal(SIGTERM, shandler);

    srand(time(NULL));

    /* 2 tiles in the beginning */
    add_tiles();
    add_tiles();

    while (1) {
        if (won) {
            printf("You win. Final score: %5d\n", score);
            goto exit;
        } else {
            printf("\033[2J\033[H"); /* clear screen */
            printf("Score: %5d\n\n", score);
        }
        if (nmoves)
            add_tiles();
        nmoves = 0;
        display();
        if (!move_possible()) {
            printf("You lose.\n");
            goto exit;
        }
        c = getchar();
        switch (c) {
        case 'q':
            goto exit;
            break;
        case 'w':
        case '8':
            shift(0, 0);
            break;
        case 'a':
        case '4':
            shift(1, 0);
            break;
        case 's':
        case '5':
            shift(0, 1);
            break;
        case 'd':
        case '6':
            shift(1, 1);
            break;
        default:
            break;
        }
    }

exit:
    reset_termios();
    return ret;
}

