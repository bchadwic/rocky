#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include "../include/peer.h"

#define SOCKET_ADDR_INPUT_SIZE 21
#define MAX_IP_INPUT_SIZE 16
#define MAX_PORT_INPUT_SIZE 5

int getpeeraddress(struct sockaddr_in *addr)
{
    printf("peer: ");
    char buf[SOCKET_ADDR_INPUT_SIZE + 1] = {0};
    if (fgets(buf, sizeof(buf), stdin) == NULL)
    {
        return -1;
    }

    char *colon = strchr(buf, ':');
    if (!colon)
    {
        return -1; // TODO; create custom error code
    }

    char ip[16];
    char port[6];

    size_t n = colon - buf;
    strncpy(ip, buf, n);
    ip[n] = '\0';

    strncpy(port, colon + 1, sizeof(port) - 1);
    port[sizeof(port) - 1] = '\0';

    addr->sin_family = AF_INET;
    addr->sin_port = atoi(port);
    if (addr->sin_port == 0)
    {
        return -1; // TODO; create custom error code
    }

    if (inet_pton(AF_INET, ip, &addr->sin_addr.s_addr) != 1)
    {
        return -1;
    }
    return 0;
}