#ifndef HOLEPUNCH_H
#define HOLEPUNCH_H

#include <netinet/in.h>

#define BURST_COUNT 3
#define CONNECT_TIMEOUT 3        // 3s
#define RAPID_INTERVAL 10000000  // 10ms
#define RETRY_INTERVAL 200000000 // 200ms

int holepunch(int fd, struct sockaddr_in *peer_addr);

#endif