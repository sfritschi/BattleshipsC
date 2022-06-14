#include "battle.h"
#include "communicate.h"

#define PORT "8888"

// PRE: Socket of peer and send buffer + length
// POST: Blocks until all data has been successfully sent
int send_full(const int socket_peer, const void *buf, int message_len) {
    int begin = 0;
    int bytes_sent;
    
    while (begin < message_len) {
        bytes_sent = send(socket_peer, (void *)((char *)buf + begin), 
            message_len - begin, 0);
        if (bytes_sent < 0) {
            return bytes_sent;  // error
        }
        begin += bytes_sent;
    }
    // DEBUG
    assert(begin == message_len);
    
    return begin;
}

// PRE: Socket of peer and receive buff + length
// POST: Blocks until all data has been successfully received
int recv_full(const int socket_peer, const void *buf, int message_len) {
    int begin = 0;
    int bytes_recv;
    
    while (begin < message_len) {
        bytes_recv = recv(socket_peer, (void *)((char *)buf + begin), 
            message_len - begin, 0);
        if (bytes_recv <= 0) {
            return bytes_recv;  // error/shutdown
        }
        begin += bytes_recv;
    }
    // DEBUG
    assert(begin == message_len);
    
    return begin;
}

// PRE: Name of host in network
// POST: Returns ip address (string)
int hostname_to_ip(const char *hostname, char *ipstr) {
	struct addrinfo hints, *res;
	int status;
	
	// Initialize hints
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;  // IPv4
	
	if ((status = getaddrinfo(hostname, NULL, &hints, &res))) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
		return status;
	}
    
	if ((status = getnameinfo(res->ai_addr, res->ai_addrlen, ipstr,
            INET_ADDRSTRLEN, NULL, 0, NI_NUMERICHOST))) {
        fprintf(stderr, "getnameinfo: %s\n", gai_strerror(status));
        return status;
    }
	// clean-up
	freeaddrinfo(res);
	
	return 0;	
}

// PRE: Connects host (server) with joinee (client)
// POST: Returns 0 on success, 1 otherwise
int connect_players(int *socket_listen, int *socket_peer, enum MODE mode) {
    assert(socket_listen != NULL && socket_peer != NULL);
    int status;
    
	if (mode == HOST) {
		// Fetch hostname
		char my_hostname[HOST_NAME_MAX];
		
		if (gethostname(my_hostname, HOST_NAME_MAX) != 0) {
			fprintf(stderr, "Could not fetch hostname\n");
			return 1;
		}
		printf("Hosting from: %s\n", my_hostname);
		
        struct addrinfo hints;
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_INET;  // IPv4
        hints.ai_socktype = SOCK_STREAM;  // TCP
        hints.ai_flags = AI_PASSIVE;  // suitable for binding
        
        struct addrinfo *bind_address;
        
        if ((status = getaddrinfo(NULL, PORT, &hints, &bind_address))) {
            fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
            return status;
        }
        
		// Initializing socket
		*socket_listen = socket(bind_address->ai_family, 
            bind_address->ai_socktype, bind_address->ai_protocol);
            
		if (*socket_listen < 0) {
			perror("Failed to create socket");
            return *socket_listen;
		}
		printf("Socket created\n");
		
		// Bind socket
		if ((status = bind(*socket_listen, bind_address->ai_addr, 
                bind_address->ai_addrlen)) < 0) {
			perror("Failed to bind socket");
			return status;
		}
        // Free resources
        freeaddrinfo(bind_address);
		printf("Bind done\n");
		
		// Listen for 1 client
		if (listen(*socket_listen, 1) < 0) {
            perror("Listen failed. Error");
            return 1;
        }
		
		// Accept any incoming connection
		printf("Waiting for opponent to join...\n");
		
        struct sockaddr_storage client_address;
        socklen_t client_len = sizeof(client_address);
        
		*socket_peer = accept(*socket_listen, (struct sockaddr *)&client_address,
            &client_len);
            
		if (*socket_peer < 0) {
			perror("Failed to accept client");
			return *socket_peer;
		}
		printf("Connection successful\n");
		
	} else if (mode == JOIN) {
		char hostname[HOST_NAME_MAX];
		char ipstr[INET_ADDRSTRLEN];
		
		printf("Enter hostname: ");
		while (!is_valid_input(scanf("%s", hostname), 1));
        // Find IP address of host
        if ((status = hostname_to_ip(hostname, ipstr))) {
			return status;
		}
		printf("Host found at: %s\n", ipstr);
        
        struct addrinfo hints;
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        
        struct addrinfo *peer_address;
        
        if ((status = getaddrinfo(ipstr, PORT, &hints, &peer_address))) {
            fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
            return status;
        }
        
		*socket_peer = socket(peer_address->ai_family, 
            peer_address->ai_socktype, peer_address->ai_protocol);
        
		if (*socket_peer < 0) {
			perror("Failed to create socket");
            return *socket_peer;
		}
		printf("Socket created\n");
		
		// Connect to host
		if ((status = connect(*socket_peer, peer_address->ai_addr, 
                peer_address->ai_addrlen) < 0)) {
			perror("Connect failed. Error");
			return status;
		}
        // Free resources
        freeaddrinfo(peer_address);
		printf("Connected to host\n");
	}
	return 0;
}

// PRE: Send 'send buffer' to opponent and receive opponent buffer in
//      Receive buffer
// POST: 0 on success 1 on error/shutdown
int sendrecv(const int socket_peer, const void *send_buf, 
        const void *recv_buf, int message_size, enum MODE mode) {
	if (mode == HOST) {
		printf("Sending to opponent...\n");
		if (send_full(socket_peer, send_buf, message_size) < 0) {
			perror("Send failed");
			return 1;
		}
		printf("Waiting for opponent...\n");
		if (recv_full(socket_peer, recv_buf, message_size) <= 0) {
			perror("Recv failed");
			return 1;
		}
	} else {
		printf("Sending to opponent...\n");
		if (send_full(socket_peer, send_buf, message_size) < 0) {
			perror("Send failed");
			return 1;
		}
		printf("Waiting for opponent...\n");
		if (recv_full(socket_peer, recv_buf, message_size) <= 0) {
			perror("Receive failed");
			return 1;
		}
	}
	return 0;
}

// PRE: Exchange shots between player and opponent
// POST: Returns 1 on error and 0 otherwise
int exchange_shots(const int socket_peer, int coord_size, 
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
		if (recv_full(socket_peer, opponent_coords, coord_size) <= 0) {
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
		if (send_full(socket_peer, player_coords, coord_size) < 0) {
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
		if (send_full(socket_peer, player_coords, coord_size) < 0) {
			perror("Send failed");
			return 1;
		}
		printf("Waiting for opponent's move...\n");
		if (recv_full(socket_peer, opponent_coords, coord_size) <= 0) {
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
