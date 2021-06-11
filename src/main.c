#include "battle.h"
#include "communicate.h"

// Global variables to keep track of game progress
int player_ship_count = NUM_SHIP_PARTS;
int player_score = 0;

int opponent_ship_count = NUM_SHIP_PARTS;
int opponent_score = 0;

int main(int argc, char *argv[]) {
	
	if (argc != 2) {
		fprintf(stderr, "Usage: ./battle <h(ost), j(oin)>\n");
		return 1;
	}
	
	int socket_serv, socket_cl;
	struct sockaddr_in server, client;
	const size_t board_message_size = BOARD_SIZE * sizeof(char);
	const size_t map_message_size = BOARD_SIZE * sizeof(int);
	const size_t coords_message_size = 2 * sizeof(int);
	
	// Connect host (server) with client
	char mode = *(argv[1]);
	if (mode != HOST && mode != JOIN) {
		fprintf(stderr, "Unrecognized mode; must be either h or j\n");
		return 1;
	}
	if (connect_players(&socket_serv, &socket_cl, &server, &client, mode) != 0) {
		return 1;
	}
	
	// Initialize board and map of ships (indices in array of ships)
	int player_map[BOARD_SIZE], opponent_map[BOARD_SIZE];
	char player_board[BOARD_SIZE], opponent_board[BOARD_SIZE];
	
beginning:
	init(player_board, player_map);
	
	// Both players individually place ships
	place_all_ships(player_board, player_map);
	
	// Exchange boards AND ship maps
	printf("Exchanging player data\n");
	if (sendrecv(socket_cl, player_board, opponent_board, board_message_size, mode) != 0) {
		return 1;
	}
	if (sendrecv(socket_cl, player_map, opponent_map, map_message_size, mode) != 0) {
		return 1;
	}
	printf("Exchange done\n");
	
	
	// Draw player and opponent board next to eachother
	draw_board_side_by_side(player_board, opponent_board, PLAYING);
	
	// Shoot coordinates message buffers
	int player_coords[2], opponent_coords[2];
	// Game loop
	while (1) {
		int err = exchange_shots(socket_cl, coords_message_size, 
                   player_coords, player_board, player_map, 
                   player_ships, &player_ship_count, 
                   opponent_coords, opponent_board, opponent_map,
                   opponent_ships, &opponent_ship_count,
                   mode);
		if (err != 0) {
			// Error occured -> exit
			break;
		}
		
		if (player_ship_count == 0 || opponent_ship_count == 0) {
			// Print updated board (with opponent ships)
			draw_board_side_by_side(player_board, opponent_board, GAMEOVER);
			break;
		} else {
			// Print updated board (without opponent ships)
			draw_board_side_by_side(player_board, opponent_board, PLAYING);
		}
	}
	// Check if error occurred
	if (player_ship_count != 0 && opponent_ship_count != 0) {
		printf("Connection was interrupted\n");
		close(socket_cl);
		return 1;
	}
	// Determine who won
	if (player_ship_count == 0 && opponent_ship_count == 0) {
		printf("DRAW! :|\n");
		opponent_score += 1;
		player_score += 1;
	} else if (player_ship_count == 0) {
		printf("YOU LOST! :(\n");
		opponent_score += 3;
	} else {
		printf("YOU WON! :)\n");
		player_score += 3;
	}
	printf("Your score: %d\n", player_score);
	printf("Opponent score: %d\n", opponent_score);
	
	// Ask both players if they want a rematch
	char player_reply;
	char opponent_reply;
	size_t reply_size = sizeof(char);
	
	printf("Do you want a rematch? [y/n]: ");
	while(!is_valid_input(scanf("%*c%c", &player_reply), 1));
	
	if (sendrecv(socket_cl, &player_reply, &opponent_reply, reply_size, mode) != 0) {
		return 1;
	}
		
	if (player_reply == 'y' && opponent_reply == 'y') {
		printf("\nStarting rematch...\n");
		// Reset counters
		player_ship_count = NUM_SHIP_PARTS;
		opponent_ship_count = NUM_SHIP_PARTS;
		// Go back to beginning
		goto beginning;
	}
	
	// Print final game message
	if (player_score < opponent_score) {
		printf("YOU LOST THE GAME. BETTER LUCK NEXT TIME!\n");
	} else if (player_score > opponent_score) {
		printf("YOU ARE THE OVERALL WINNER! CONGRATS!\n");
	} else {
		printf("DRAW!\n");
	}
	
	// Close socket
	close(socket_cl);
	
	return 0;
}


