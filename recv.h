#ifndef RECV_H

// fopen()
#include <stdio.h>
// open()
#include <sys/socket.h>
// close()
#include <unistd.h>
// IPPROTO_UDP
#include <netinet/in.h>

#define ODON_PORT 52899

int odon_recv(char *r_file);

#endif