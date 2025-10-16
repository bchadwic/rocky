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
#include "../include/holepunch.h"

static int burst(int fd, struct sockaddr_in *peer_addr)
{
    printf("sending initial packets\n");
    struct timespec ts = {.tv_sec = 0, .tv_nsec = RAPID_INTERVAL};

    for (int i = 1; i <= 5; i++)
    {
        if (sendto(fd, "ping", 4, 0, (struct sockaddr *)peer_addr, sizeof(*peer_addr)) == -1)
        {
            return -1;
        }
        printf("packet %d sent\n", i);
        if (nanosleep(&ts, NULL) == -1)
        {
            return -1;
        }
    }
    return 0;
}

static int tryconnect(int epfd, int fd, struct sockaddr_in *peer_addr)
{
    struct epoll_event ev = {.events = EPOLLIN | EPOLLET | EPOLLONESHOT, .data = 0};
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, 0) == -1)
    {
        return -1;
    }

    bool connected = false;
    while (!connected)
    {
        if (sendto(fd, "ping", 4, 0, (struct sockaddr *)peer_addr, sizeof(*peer_addr)) == -1)
        {
            return -1;
        }
    }
    return 0;
}

int holepunch(int fd, struct sockaddr_in *peer_addr)
{
    if (burst(fd, peer_addr) == -1)
    {
        return -1;
    }

    int epfd = epoll_create(1); // poll just our socket, size ignored since Linux 2.6.8
    int connected = tryconnect(epfd, fd, peer_addr);
    close(epfd);

    if (connected == -1)
    {
        return -1;
    }

    return 0;
}

// FD_ZERO(&readfds);
// FD_SET(fd, &readfds);

// struct timeval timeout = {.tv_sec = 5, .tv_usec = 0};
// // int ready = ;
// switch(select(fd + 1, &readfds, NULL, NULL, &timeout)){
//     case 3:

// }

// if (ready > 0 && FD_ISSET(fd, &readfds))
// {
//     struct sockaddr_in from;
//     socklen_t fromlen = sizeof(from);
//     ssize_t len = recvfrom(fd, buf, sizeof(buf) - 1, 0, (struct sockaddr *)&from, &fromlen);
//     if (len > 0)
//     {
//         buf[len] = '\0';
//         printf("Received from %s:%d → %s\n", inet_ntoa(from.sin_addr), ntohs(from.sin_port), buf);
//         connected = true;
//     }
// }
// else
// {
//     printf("no response, retrying...\n");
//     struct timespec ts = {.tv_sec = 0, .tv_nsec = RETRY_INTERVAL};
//     if (nanosleep(&ts, NULL) == -1)
//     {
//         return -1;
//     }
// }