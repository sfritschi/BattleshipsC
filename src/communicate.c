#include "battle.h"
#include "communicate.h"

#define PORT (8888)

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

// PRE: Connects host (server) with joinee (client)
// POST: Returns 0 on success, 1 otherwise
int connect_players(int *socket_serv, int *socket_cl, 
                    struct sockaddr_in *server,
                    struct sockaddr_in *client, enum MODE mode) {
	if (mode == HOST) {
		// Fetch hostname
		char my_hostname[HOST_NAME_MAX];
		
		if (gethostname(my_hostname, HOST_NAME_MAX) != 0) {
			fprintf(stderr, "Could not fetch hostname\n");
			return 1;
		}
		printf("Hosting from: %s\n", my_hostname);
		
		// Initializing socket
		*socket_serv = socket(AF_INET, SOCK_STREAM, 0);
		if (*socket_serv == -1) {
			fprintf(stderr, "Could not create socket\n");
			return 1;
		}
		printf("Socket created\n");
		
		memset(server, 0, sizeof(server[0]));
		server->sin_family = AF_INET;
		server->sin_addr.s_addr = INADDR_ANY;
		server->sin_port = htons( PORT );
		
		// Bind socket
		if (bind(*socket_serv, (struct sockaddr *)server, sizeof(server[0])) < 0) {
			perror("Bind failed. Error");
			return 1;
		}
		printf("Bind done\n");
		
		// Listen for 1 client
		if (listen(*socket_serv, 1) < 0) {
            perror("Listen failed. Error");
            return 1;
        }
		
		// Accept any incoming connection
		printf("Waiting for opponent to join...\n");
		int c;
		
		*socket_cl = accept(*socket_serv, (struct sockaddr *)client, (socklen_t *)&c);
		if (*socket_cl < 0) {
			perror("Accept failed");
			return 1;
		}
		printf("Connection successful\n");
		
	} else if (mode == JOIN) {
		char hostname[HOST_NAME_MAX];
		char ipstr[INET_ADDRSTRLEN];
		
		printf("Enter hostname: ");
		while (!is_valid_input(scanf("%s", hostname), 1));
		
		*socket_cl = socket(AF_INET, SOCK_STREAM, 0);
		if (*socket_cl == -1) {
			printf("Could not create socket\n");
		}
		printf("Socket created\n");
		
		if (hostname_to_ip(hostname, ipstr) != 0) {
			return 1;
		}
		printf("Host found at: %s\n", ipstr);
		
		memset(server, 0, sizeof(server[0]));
		server->sin_addr.s_addr = inet_addr(ipstr);
		server->sin_family = AF_INET;
		server->sin_port = htons( PORT );
		
		// Connect to host
		if (connect(*socket_cl, (struct sockaddr *)server, sizeof(server[0])) < 0) {
			perror("Connect failed. Error");
			return 1;
		}
		printf("Connected to host\n");
	}
	return 0;
}

// PRE: Send 'send buffer' to opponent and receive opponent buffer in
//      Receive buffer
// POST: -
int sendrecv(const int socket_cl, void *send_buf, void *recv_buf,
             size_t message_size, enum MODE mode) {
	if (mode == HOST) {
		printf("Sending to opponent...\n");
		if (write(socket_cl, send_buf, message_size) < 0) {
			perror("Send failed");
			return 1;
		}
		printf("Waiting for opponent...\n");
		if (recv(socket_cl, recv_buf, message_size, 0) <= 0) {
			perror("Recv failed");
			return 1;
		}
	} else {
		printf("Sending to opponent...\n");
		if (send(socket_cl, send_buf, message_size, 0) < 0) {
			perror("Send failed");
			return 1;
		}
		printf("Waiting for opponent...\n");
		if (recv(socket_cl, recv_buf, message_size, 0) <= 0) {
			perror("Receive failed");
			return 1;
		}
	}
	return 0;
}

// PRE: Exchange shots between player and opponent
// POST: Returns 1 on error and 0 otherwise
int exchange_shots(const int socket_cl, size_t coord_size, 
                   int *player_coords, char *player_board, const int *player_map, 
                   struct ship_t *player_ships, int *player_ship_count, 
                   int *opponent_coords, char *opponent_board, const int *opponent_map,
                   struct ship_t *opponent_ships, int *opponent_ship_count,
                   enum MODE mode) {
    int row, col;
    int opp_row, opp_col;
    int is_hit;
    
	if (mode == HOST) {
		printf("Waiting for opponent's move...\n");
		if (recv(socket_cl, opponent_coords, coord_size, 0) <= 0) {
			perror("Target recv failed");
			return 1;
		}
		opp_row = opponent_coords[0]; opp_col = opponent_coords[1];
		// Shoot own board
		is_hit = shoot(opp_row, opp_col, player_board, player_map, 
		               player_ship_count, player_ships, SELF);
		// Print results
		print_results(opp_row, opp_col, is_hit, OPPONENT);
	
		printf("Enter shoot coords: ");
		while(!is_valid_input(scanf("%d %d", &row, &col), 2));
		// Shoot opponent board
		while((is_hit = shoot(row, col, opponent_board, opponent_map,
		                      opponent_ship_count, opponent_ships, OPPONENT)) == -1) {
			printf("Invalid coordinates, try again: ");
			while(!is_valid_input(scanf("%d %d", &row, &col), 2));
		}
		player_coords[0] = row;
		player_coords[1] = col;
		// Print results
		print_results(row, col, is_hit, SELF);
		// Send shoot coordinates to opponent
		if (write(socket_cl, player_coords, coord_size) < 0) {
			perror("Send failed");
			return 1;
		}
	} else {  // Client shoots first			
		printf("Enter shoot coords: ");
		while(!is_valid_input(scanf("%d %d", &row, &col), 2));
		// Shoot opponent board
		while((is_hit = shoot(row, col, opponent_board, opponent_map,
		                      opponent_ship_count, opponent_ships, OPPONENT)) == -1) {
			printf("Invalid coordinates, try again: ");
			while(!is_valid_input(scanf("%d %d", &row, &col), 2));
		}
		player_coords[0] = row;
		player_coords[1] = col;
		// Print results
		print_results(row, col, is_hit, SELF);
		// Send shoot coordinates to opponent
		if (send(socket_cl, player_coords, coord_size, 0) < 0) {
			perror("Send failed");
			return 1;
		}
		printf("Waiting for opponent's move...\n");
		if (recv(socket_cl, opponent_coords, coord_size, 0) <= 0) {
			perror("Target recv failed");
			return 1;
		}
		opp_row = opponent_coords[0]; opp_col = opponent_coords[1];
		// Shoot own board
		is_hit = shoot(opp_row, opp_col, player_board, player_map,
		               player_ship_count, player_ships, SELF);
		// Print results
		print_results(opp_row, opp_col, is_hit, OPPONENT);
	}
	return 0;
}
