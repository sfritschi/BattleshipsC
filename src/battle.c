#include "battle.h"

#include <string.h>

// Global array of ships for both player and opponent
struct ship_t player_ships[NUM_SHIPS] = {
	{"CARRIER", 5, 0},
	{"BATTLESHIP", 4, 0},
	{"CRUISER", 3, 0},
	{"SUBMARINE", 3, 0},
	{"DESTROYER", 2, 0}
};
struct ship_t opponent_ships[NUM_SHIPS] = {
	{"CARRIER", 5, 0},
	{"BATTLESHIP", 4, 0},
	{"CRUISER", 3, 0},
	{"SUBMARINE", 3, 0},
	{"DESTROYER", 2, 0}
};

// PRE: Checks if input is valid by comparing against expected value
// POST: -
int is_valid_input(const int given, const int expected) {
	if (given != expected) {
		printf("Expected exactly %d argument(s)\n", expected);
		int ans = scanf("%*s");  // Ignore invalid input
		(void) ans;  // Don't warn me
		return 0;
	}
	return 1;
}

// PRE: Print a character in a certain color to console
// POST: -
void print_char_col(const char c, const unsigned int color) {
	printf("\033[1;%dm%c\033[0m", color, c); 
}

// PRE: Prints symbol in defined color
//      - Ship 'S': Yellow
//      - Water '*': Blue
//      - Miss 'O': Cyan
//      - Hit 'X': Red
// POST: -
void print_symbol(const char c) {
	switch (c) {
		case SHIP:
		print_char_col(c, YELLOW);
		break;
		
		case WATER:
		print_char_col(c, BLUE);
		break;
		
		case MISS:
		print_char_col(c, CYAN);
		break;
		
		default:  // 'X'
		print_char_col(c, RED);
	}	
}

// PRE: Print a string in a certain color to console
// POST: -
void print_str_col(const char *str, const unsigned int color) {
	printf("\033[1;%dm%s\033[0m", color, str); 
}

// PRE: Print results of shots fired at individual boards
// POST: -
void print_results(const int row, const int col, const int is_hit, 
                   enum PLAYER player_type) {
	
	if (player_type == SELF) {
	    printf("You shot: (%d, %d) ", row, col);
	} else {
	    printf("Opponent shot: (%d, %d) ", row, col);
	}
	
	if (is_hit) {
		print_str_col("HIT!", RED);
	} else {
		print_str_col("MISS!", CYAN);
	}
	printf("\n");
}

// PRE: Fills player board with '*' (water)
// POST: Initalized board
void init(char *player_board, int *player_map) {
	int i;
	for (i = 0; i < BOARD_SIZE; ++i) {
		player_board[i] = WATER;
		
		player_map[i] = -1;  // No ship present
	}	
}

// PRE: Check if coordinates (row, col) lie inside board
// POST: 1 if coordinates lie inside, 0 otherwise
int is_inside(const int r, const int c) {
	return (0 <= r && r < BOARD_LENGTH) && (0 <= c && c < BOARD_LENGTH);
}

// PRE: Based on orientation, check if ship overlaps already existing
//      ships
// POST: 1 if this is the case, 0 otherwise
int is_overlap(const char *board, const int length, 
                     const int r, const int c, enum ORIENTATIONS o) {
	int i;
	if (o == HORIZONTAL) {
		for (i = 0;	i < length; ++i) {
			if (board[r * BOARD_LENGTH + c + i] == SHIP) {
				return 1;
			}
		}
	} else {
		for (i = 0; i < length; ++i) {
			if (board[(r + i) * BOARD_LENGTH + c] == SHIP) {
				return 1;
			}
		}
	}
	return 0;
}

// PRE: Shoot opponent board at given coordinates
// POST: -1 if target coordinates were invalid, 0 if MISS 1 if HIT
int shoot(const int row, const int col, char *board, const int *map,
                int *counter, struct ship_t *ships, enum PLAYER player_type) {
	const int r = row - 1;	
	const int c = col - 1;
		
	if (is_inside(r, c)) {
		const int index = r * BOARD_LENGTH + c;
		const char target = board[index];
		
		// Make sure location hasn't been shot already
		if (target == SHIP) {
			// Shoot! Indicate whether the ship was hit or missed
			board[index] = HIT;
			// Check which ship was hit
			const int ship_id = map[index];
			struct ship_t *target_ship = &ships[ship_id];
			// Increment hit counter of ship
			int hits = ++target_ship->hits_taken;
			// Check if ship was destroyed
			if (hits == target_ship->length) {
				if (player_type == SELF)
					printf("Your %s has been destroyed!\n", target_ship->name);
				else
				    printf("Enemy %s has been destroyed!\n", target_ship->name);
			}
			// Update counter
			(*counter)--;
			
			return 1;  // ok
		} else if (target == WATER) {
			board[index] = MISS;
			
			return 0;  // ok
		}
		
	}
	return -1;
}

// PRE: Draws board to console
// POST: -
void draw_board(const char *board) {
	const char separator[] = "-----------------------------------------";
	
	// Print header
	print_char_col('r', GREEN);
	printf("\\");
	print_char_col('c', MAGENTA);
	printf(" ");
	
	int i, j;
	int row = 1, col;
	for (col = 1; col <= BOARD_LENGTH; ++col) {
		printf("%2d  ", col);
	} 
	printf("\n");
	
	char c;
	// Print board
	for (i = 0; i < BOARD_LENGTH; ++i) {
		printf("   %s\n", separator);
		printf("%2d | ", row);
		for (j = 0; j < BOARD_LENGTH - 1; ++j) {
			c = board[i * BOARD_LENGTH + j];
			print_symbol(c);
			printf(" | ");
		}
		c = board[i * BOARD_LENGTH + BOARD_LENGTH - 1];
		print_symbol(c);
		printf(" |");
		printf("\n");
		
		row++;		
	}
	printf("   %s\n", separator);
}

// PRE: Draws player board (left) next to opponent board (right)
// POST: -
void draw_board_side_by_side(const char *player_board, 
                             const char *opponent_board,
                             enum STATE game_state) {
	const char separator[] = "-----------------------------------------";
	const char line[] = "   |   ";
	// Print header
	print_char_col('r', GREEN);
	printf("\\");
	print_char_col('c', MAGENTA);
	printf(" ");
	
	int i, j;
	int row, col;
	for (col = 1; col <= BOARD_LENGTH; ++col) {
		printf("%2d  ", col);
	}
	printf("%s ", line);
	for (col = 1; col <= BOARD_LENGTH; ++col) {
		printf("%2d  ", col);
	}
	print_char_col('c', MAGENTA);
	printf("/");
	print_char_col('r', GREEN);
	
	printf("\n");
	
	char c;
	// Print board
	for (i = 0, row = 1; i < BOARD_LENGTH; ++i) {
		printf("   %s%s%s\n", separator, line, separator);
		printf("%2d | ", row);
		for (j = 0; j < BOARD_LENGTH - 1; ++j) {
			c = player_board[i * BOARD_LENGTH + j];
			print_symbol(c);
			printf(" | ");
		}
		c = player_board[i * BOARD_LENGTH + BOARD_LENGTH - 1];
		print_symbol(c);
		
		printf(" |");
		printf("%s| ", line);
		for (j = 0; j < BOARD_LENGTH - 1; ++j) {
			c = opponent_board[i * BOARD_LENGTH + j];
			if (game_state == PLAYING) {
				c = (c == SHIP) ? WATER : c;  // don't print opponent ships
		    }
			print_symbol(c);
			printf(" | ");
		}
		c = opponent_board[i * BOARD_LENGTH + BOARD_LENGTH - 1];
		if (game_state == PLAYING) {
			c = (c == SHIP) ? WATER : c;
		}
		print_symbol(c);
		printf(" |");
		
		printf(" %d ", row);
		printf("\n");
		
		row++;		
	}
	printf("   %s%s%s\n", separator, line, separator);
}

// PRE: Place all ships within board given player input
// POST: -
void place_ship(const struct ship_t *ship, char *player_board,
                int *player_map, const int ship_id) {
	printf("Placing ship of type %s:\n", ship->name);
	printf("Enter orientation [h,v]: ");
	char orientation;
	while(!is_valid_input(scanf("%c", &orientation), 1));
	while (orientation != HORIZONTAL && orientation != VERTICAL) {
		while(!is_valid_input(scanf("%c", &orientation), 1));
	}
	
	int row, col;
	printf("Enter: ");
	print_str_col("row", GREEN);
	printf(" ");
	print_str_col("col", MAGENTA); 
	printf(" of origin (length: %d): ", ship->length);
	while(!is_valid_input(scanf("%d %d", &row, &col), 2));
	
	// zero-based
	row = row - 1;
	col = col - 1;
	
	int i;
	int index;
	if (orientation == HORIZONTAL) {
		// Make sure current ship lies within board
		while (col < 0 || !is_inside(row, col + ship->length - 1)) {
			printf("\nShip is outside of bounds, try again: ");
			while(!is_valid_input(scanf("%d %d", &row, &col), 2));
			// zero-based
	        row = row - 1;
	        col = col - 1;
		}
		// Make sure current ship does not overlap with previous ships
		while (is_overlap(player_board, ship->length, row, col, HORIZONTAL)) {
			printf("\nShips overlap, try again: ");
			while(!is_valid_input(scanf("%d %d", &row, &col), 2));
			// zero-based
	        row = row - 1;
	        col = col - 1;
		}
		index = row * BOARD_LENGTH + col;
		// Place ship
		for (i = 0; i < ship->length; ++i) {
			player_board[index + i] = SHIP;
			player_map[index + i] = ship_id;
		}
	} else {
		// Make sure current ship lies within board
		while (row < 0 || !is_inside(row + ship->length - 1, col)) {
			printf("\nShip is outside of bounds, try again: ");
			while(!is_valid_input(scanf("%d %d", &row, &col), 2));
			// zero-based
	        row = row - 1;
	        col = col - 1;
		}
		// Make sure current ship does not overlap with previous ships
		while (is_overlap(player_board, ship->length, row, col, VERTICAL)) {
			printf("\nShips overlap, try again: ");
			while(!is_valid_input(scanf("%d %d", &row, &col), 2));
			// zero-based
	        row = row - 1;
	        col = col - 1;
		}
		index = row * BOARD_LENGTH + col;
		// Place ship
		for (i = 0; i < ship->length; ++i) {
			player_board[index] = SHIP;
			player_map[index] = ship_id;
			
			index += BOARD_LENGTH;
		}
	}
}

// PRE: Based on user input, place all ships in board
// POST: -
void place_all_ships(char *player_board, int *player_map) {
	// Draw board
	draw_board(player_board);
	
	int i;
	for (i = 0; i < NUM_SHIPS; ++i) {
		const struct ship_t current = player_ships[i];
		place_ship(&current, player_board, player_map, i);
			
		draw_board(player_board);
	}
}
