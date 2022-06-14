#ifndef COMMUNICATE_H
#define COMMUNICATE_H

#include <stdlib.h>
#include <string.h>  // memset
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <limits.h>
#include <assert.h>

// PRE: Socket of peer and send buffer + length
// POST: Blocks until all data has been successfully sent
int send_full(const int, const void *, int);

// PRE: Socket of peer and receive buff + length
// POST: Blocks until all data has been successfully received
int recv_full(const int, const void *, int);

// PRE: Name of host in network
// POST: Returns ip address (string)
int hostname_to_ip(const char *, char *);

// PRE: Connects host (server) with joinee (client)
// POST: Returns 0 on success, 1 otherwise
int connect_players(int *, int *, enum MODE);

// PRE: Send 'send buffer' to opponent and receive opponent buffer in
//      'receive buffer'
// POST: -
int sendrecv(const int, const void *, const void *, int, enum MODE);

// PRE: Exchange shots between player and opponent
// POST: Returns 1 on error and 0 otherwise
int exchange_shots(const int, int, 
                   int *, char *, const int *, 
                   struct ship_t *, int *, 
                   int *, char *, const int *,
                   struct ship_t *, int *,
                   enum MODE mode);

#endif /* COMMUNICATE_H */
