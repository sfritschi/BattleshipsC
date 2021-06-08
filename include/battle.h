#ifndef BATTLE_H
#define BATTLE_H

#include <stdio.h>

#define BOARD_LENGTH (10)
#define BOARD_SIZE (100)
#define NUM_SHIPS (5)
#define NUM_SHIP_PARTS (17)

// Colors used for symbols
enum COLORS {
	DEFAULT_WHITE = 0,
	YELLOW = 33,  // Ship 'S'
	BLUE = 34,    // Water '*'
	CYAN = 36,    // Miss 'O'
	RED = 31,     // Hit 'X'
	GREEN = 32,   // row
	MAGENTA = 35  // column
};

// Symbols used for different objects
enum SYMBOLS {
	SHIP = 'S',
	WATER = '*',
	MISS = 'O',
	HIT = 'X'
};

enum ORIENTATIONS {
	HORIZONTAL = 'h',
	VERTICAL = 'v'
};

enum STATE {
	GAMEOVER,
	PLAYING
};

// Ship datatype
struct ship_t {
	char *name;
	int length;
	int amount;
};

// PRE: Checks if input is valid by comparing against expected value
// POST: -
int is_valid_input(const int, const int);

// PRE: Print a character in a certain color to console
// POST: -
void print_char_col(const char, const unsigned int);

// PRE: Prints symbol in defined color
//      - Ship 'S': Yellow
//      - Water '*': Blue
//      - Miss 'O': Cyan
//      - Hit 'X': Red
// POST: -
void print_symbol(const char);

// PRE: Print a string in a certain color to console
// POST: -
void print_str_col(const char *, const unsigned int);

// PRE: Print results of shots fired at individual boards
// POST: -
void print_results(const int, const int, const char *,
                   const int, const int, const char *,
                   const int, const int);

// PRE: Fills player board with '*' (water)
// POST: Initalized board
void init_board(char *);

// PRE: Check if coordinates (row, col) lie inside board
// POST: 1 if coordinates lie inside, 0 otherwise
int is_inside(const int, const int);

// PRE: Based on orientation, check if ship overlaps already existing
//      ships
// POST: 1 if this is the case, 0 otherwise
int is_overlap(const char *, const int, 
                     const int, const int, enum ORIENTATIONS);

// PRE: Shoot opponent board at given coordinates
// POST: 1 if target coordinates were invalid, 0 if ok
int shoot(const int, const int, char *, int *);

// PRE: Draws board to console
// POST: -
void draw_board(const char *);

// PRE: Draws player board (left) next to opponent board (right)
// POST: -
void draw_board_side_by_side(const char *, 
                             const char *,
                             enum STATE);

// PRE: Place all ships within board given player input
// POST: -
void place_ship(const struct ship_t *, const int, char *);

// PRE: Based on user input, place all ships in board
// POST: -
void place_all_ships(char *);

#endif /* BATTLE_H */
