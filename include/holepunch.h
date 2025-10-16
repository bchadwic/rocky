#ifndef HOLEPUNCH_H
#define HOLEPUNCH_H

#include <netinet/in.h>

#define RAPID_INTERVAL 10000000  // 10ms
#define RETRY_INTERVAL 200000000 // 200ms

// const char *PING = "ping";
// const size_t PING_SIZE = 4;

int holepunch(int fd, struct sockaddr_in *peer_addr);

#endif