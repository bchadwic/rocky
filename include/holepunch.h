#ifndef CONNECT_H
#define CONNECT_H

#include <netinet/in.h>

#define RAPID_INTERVAL 10000000  // 10ms
#define RETRY_INTERVAL 200000000 // 200ms

int holepunch(int fd, struct sockaddr_in *peer_addr);

#endif