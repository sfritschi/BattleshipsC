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

// PRE: Name of host in network
// POST: Returns ip address (string)
int hostname_to_ip(const char *, char *);

// PRE: Connects host (server) with joinee (client)
// POST: Returns 0 on success, 1 otherwise
int connect_players(int *, int *, struct sockaddr_in *, 
                    struct sockaddr_in *, enum MODE);

// PRE: Send 'send buffer' to opponent and receive opponent buffer in
//      'receive buffer'
// POST: -
int sendrecv(const int, void *, void *, size_t, enum MODE);

// PRE: Exchange shots between player and opponent
// POST: Returns 1 on error and 0 otherwise
int exchange_shots(const int, size_t, 
                   int *, char *, const int *, 
                   struct ship_t *, int *, 
                   int *, char *, const int *,
                   struct ship_t *, int *,
                   enum MODE mode);

#endif /* COMMUNICATE_H */
