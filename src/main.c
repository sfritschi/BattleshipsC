#include "battle.h"

#include <stdlib.h>
#include <string.h>  // memset
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <limits.h>

#define PORT (8888)

// globals
int player_ship_count = NUM_SHIP_PARTS;
int opponent_ship_count = NUM_SHIP_PARTS;

enum MODE {
	HOST = 'h',
	JOIN = 'j'
};

// PRE: Name of host in network
// POST: Returns ip address (string)
int hostname_to_ip(const char *hostname, char *ipstr) {
	struct addrinfo hints, *res, *p;
	int status;
	
	// Initialize hints
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;  // IPv4
	hints.ai_socktype = SOCK_STREAM;
	
	if ((status = getaddrinfo(hostname, NULL, &hints, &res)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
		return status;
	}
	
	for (p = res; p != NULL; p = p->ai_next) {
		void *addr;
		struct sockaddr_in *ip = (struct sockaddr_in *)p->ai_addr;
		addr = &(ip->sin_addr);
		
		inet_ntop(AF_INET, addr, ipstr, INET_ADDRSTRLEN);
	}
	// clean-up
	freeaddrinfo(res);
	
	return 0;	
}

int main(int argc, char *argv[]) {
	
	if (argc != 2) {
		fprintf(stderr, "Usage: ./battle <h(ost), j(oin)>\n");
		return 1;
	}
	
	int socket_serv, socket_cl;
	struct sockaddr_in server, client;
	const int board_message_size = BOARD_SIZE * sizeof(char);
	const int message_size = 2 * sizeof(int);
	
	// Connect host (server) with client
	char mode = *(argv[1]);
	if (mode == HOST) {
		// Fetch hostname
		char my_hostname[HOST_NAME_MAX];
		
		if (gethostname(my_hostname, HOST_NAME_MAX) != 0) {
			fprintf(stderr, "Could not fetch hostname\n");
			return 1;
		}
		printf("Hosting from: %s\n", my_hostname);
		
		// Initializing socket
		socket_serv = socket(AF_INET, SOCK_STREAM, 0);
		if (socket_serv == -1) {
			printf("Could not create socket\n");
		}
		printf("Socket created\n");
		
		memset(&server, 0, sizeof(server));
		server.sin_family = AF_INET;
		server.sin_addr.s_addr = INADDR_ANY;
		server.sin_port = htons( PORT );
		
		// Bind socket
		if (bind(socket_serv, (struct sockaddr *)&server, sizeof(server)) < 0) {
			perror("Bind failed. Error");
			return 1;
		}
		printf("Bind done\n");
		
		// Listen for 1 client
		listen(socket_serv, 2);
		
		// Accept any incoming connection
		printf("Waiting for opponent to join...\n");
		int c;
		
		socket_cl = accept(socket_serv, (struct sockaddr *)&client, (socklen_t *)&c);
		if (socket_cl < 0) {
			perror("Accept failed");
			return 1;
		}
		printf("Connection successful\n");
		
	} else if (mode == JOIN) {
		char hostname[HOST_NAME_MAX];
		char ipstr[INET_ADDRSTRLEN];
		
		printf("Enter hostname: ");
		while (!is_valid_input(scanf("%s", hostname), 1));
		
		socket_cl = socket(AF_INET, SOCK_STREAM, 0);
		if (socket_cl == -1) {
			printf("Could not create socket\n");
		}
		printf("Socket created\n");
		
		if (hostname_to_ip(hostname, ipstr) != 0) {
			return 1;
		}
		printf("Host found\n");
		
		memset(&server, 0, sizeof(server));
		server.sin_addr.s_addr = inet_addr(ipstr);
		server.sin_family = AF_INET;
		server.sin_port = htons( PORT );
		
		// Connect to host
		if (connect(socket_cl, (struct sockaddr *)&server, sizeof(server)) < 0) {
			perror("Connect failed. Error");
			return 1;
		}
		printf("Connected to host\n");
	} else {
		fprintf(stderr, "Unrecognized mode; must be either h or j\n");
		return 1;
	}
	
	// Initialize board
	char player_board[BOARD_SIZE], opponent_board[BOARD_SIZE];
	init_board(player_board);
	
	// Both players individually place ships
	place_all_ships(player_board);
	
	// Exchange boards
	if (mode == HOST) {
		printf("Waiting for opponent to place ships...\n");
		if (recv(socket_cl, opponent_board, board_message_size, 0) <= 0) {
			perror("Recv failed");
			return 1;
		}
		printf("Received opponent board. Sending own board...\n");
		
		if (write(socket_cl, player_board, board_message_size) < 0) {
			perror("Send failed");
			return 1;
		}
	} else {
		printf("Sending board to opponent...\n");
		
		if (send(socket_cl, player_board, board_message_size, 0) < 0) {
			perror("Board send failed");
			return 1;
		}
		printf("Send complete. Waiting for opponent to finish...\n");
		
		if (recv(socket_cl, opponent_board, board_message_size, 0) <= 0) {
			perror("Board receive failed");
			return 1;
		}
	}
	printf("Board exchange done\n");
	
	// Draw player and opponent board next to eachother
	draw_board_side_by_side(player_board, opponent_board, PLAYING);
	
	int message[2];  // message buffer
	int row = 0, col = 0, opp_row = 0, opp_col = 0;
	// Game loop
	while (1) {
		// Host goes second
		if (mode == HOST) {
			printf("Waiting for opponent's move...\n");
			if (recv(socket_cl, message, message_size, 0) <= 0) {
				perror("Target recv failed");
				break;
			}
			opp_row = message[0];
			opp_col = message[1];
			
			// Shoot
			shoot(opp_row, opp_col, player_board, &player_ship_count);
			// Check if opponent has won already
			if (player_ship_count == 0) {
				goto end;
			}
			
			printf("Enter shoot coords: ");
			while(!is_valid_input(scanf("%d %d", &row, &col), 2));
			// Shoot opponent board
			while(shoot(row, col, opponent_board, &opponent_ship_count) != 0) {
				printf("\nInvalid coordinates, try again: ");
				while(!is_valid_input(scanf("%d %d", &row, &col), 2));
			}
			
			message[0] = row;
			message[1] = col;
			// Send shoot coordinates to opponent
			if (write(socket_cl, message, message_size) < 0) {
				perror("Send failed");
				break;
			}
		} else {  // Client shoots first			
			printf("Enter shoot coords: ");
			while(!is_valid_input(scanf("%d %d", &row, &col), 2));
			// Shoot opponent board
			while(shoot(row, col, opponent_board, &opponent_ship_count) != 0) {
				printf("\nInvalid coordinates, try again: ");
				while(!is_valid_input(scanf("%d %d", &row, &col), 2));
			}
			
			message[0] = row;
			message[1] = col;
			// Send shoot coordinates to opponent
			if (send(socket_cl, message, message_size, 0) < 0) {
				perror("Send failed");
				break;
			}
			
			// Check if won already
			if (opponent_ship_count == 0) {
				goto end;
			}
			printf("Waiting for opponent's move...\n");
			if (recv(socket_cl, message, message_size, 0) <= 0) {
				perror("Target recv failed");
				break;
			}
			opp_row = message[0];
			opp_col = message[1];
			
			// Shoot
			shoot(opp_row, opp_col, player_board, &player_ship_count);
		}
end:
		// Print results of shots
		print_results(row, col, player_board, opp_row, opp_col, opponent_board,
		                player_ship_count, opponent_ship_count);
		
		if (player_ship_count == 0 || opponent_ship_count == 0) {
			// Print updated board (with opponent ships)
			draw_board_side_by_side(player_board, opponent_board, GAMEOVER);
			break;
		} else {
			// Print updated board (without opponent ships)
			draw_board_side_by_side(player_board, opponent_board, PLAYING);
		}
	}
	// Determine who won
	if (player_ship_count == 0) {
		printf("YOU LOST! :(\n");
	} else if (opponent_ship_count == 0) {
		printf("YOU WON! :)\n");
	} else {
		printf("Connection was interrupted\n");
	}
	// Close sockets
	close(socket_cl);
	
	return 0;
}


