#ifndef PEER_H
#define PEER_H

#include <netinet/in.h>

int getpeeraddress(struct sockaddr_in *addr);

#endif