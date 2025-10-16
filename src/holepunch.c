#define _POSIX_C_SOURCE 199309L

#include <stdio.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <sys/random.h>
#include <netdb.h>
#include <sys/time.h>
#include <stdbool.h>
#include <time.h>
#include <sys/epoll.h>
#include <errno.h>
#include "../include/holepunch.h"

const char *PING = "ping";
const size_t PING_SIZE = 4;

static int burst(int fd, struct sockaddr_in *peer_addr)
{
    struct timespec delay = {.tv_sec = 0, .tv_nsec = RAPID_INTERVAL};

    for (int i = 1; i <= BURST_COUNT; i++)
    {
        if (sendto(fd, PING, PING_SIZE, 0, (struct sockaddr *)peer_addr, sizeof(*peer_addr)) == -1)
        {
            return -1;
        }

        printf("packet %d sent\n", i);
        if (nanosleep(&delay, NULL) == -1)
        {
            return -1;
        }
    }
    return 0;
}

static int tryconnect(int fd, struct sockaddr_in *peer_addr)
{
    // this represents which address and port our peer will communicate to us with
    struct sockaddr_in actual_peer_addr;
    socklen_t addr_len = sizeof(actual_peer_addr);
    char buf[512] = {0};
    struct timespec delay = {.tv_sec = 0, .tv_nsec = RETRY_INTERVAL};

    while (true)
    {
        if (burst(fd, peer_addr) == -1)
        {
            return -1;
        }

        ssize_t n = recvfrom(fd, buf, sizeof(buf), 0, (struct sockaddr *)&actual_peer_addr, &addr_len);
        if (n == -1)
        {
            if (errno != EAGAIN && errno != EWOULDBLOCK)
            {
                return -1;
            }
        }
        else
        {
            // received packet
            printf("received packet from peer with contents: %.*s\n", (int)n, buf);
            break;
        }

        printf("could not connect, retrying\n");
        if (nanosleep(&delay, NULL) == -1)
        {
            return -1;
        }
    }

    peer_addr->sin_addr = actual_peer_addr.sin_addr;
    peer_addr->sin_port = actual_peer_addr.sin_port;
    addr_len = sizeof(*peer_addr);
    return connect(fd, (struct sockaddr *)peer_addr, addr_len);
}

int holepunch(int fd, struct sockaddr_in *peer_addr)
{
    // set timeout so we can intervally retry
    struct timeval timeout = {.tv_sec = CONNECT_TIMEOUT, .tv_usec = 0};
    if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) == -1)
    {
        return -1;
    }

    if (tryconnect(fd, peer_addr) == -1)
    {
        return -1;
    }

    // now that we're connected, remove our timeout
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;
    if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) == -1)
    {
        return -1;
    }
    return 0;
}