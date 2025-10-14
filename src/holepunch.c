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
#include "../include/holepunch.h"

int holepunch(int fd, struct sockaddr_in *peer_addr)
{
    printf("sending initial packets\n");
    fd_set readfds;
    char buf[1024];

    bool connected = false;
    char *ping = "ping";

    // Punch NAT: send a few packets immediately
    for (int i = 0; i < 5; i++)
    {
        sendto(fd, ping, strlen(ping), 0, (struct sockaddr *)peer_addr, sizeof(*peer_addr));
        printf("packet %d sent\n", i);
        struct timespec ts = {.tv_sec = 0, .tv_nsec = RAPID_INTERVAL};
        if (nanosleep(&ts, NULL) == -1)
        {
            return -1;
        }
    }

    while (!connected)
    {
        // Send a keepalive packet every loop iteration
        sendto(fd, ping, strlen(ping), 0, (struct sockaddr *)peer_addr, sizeof(*peer_addr));

        FD_ZERO(&readfds);
        FD_SET(fd, &readfds);

        struct timeval timeout = {.tv_sec = 5, .tv_usec = 0};
        int ready = select(fd + 1, &readfds, NULL, NULL, &timeout);

        if (ready > 0 && FD_ISSET(fd, &readfds))
        {
            struct sockaddr_in from;
            socklen_t fromlen = sizeof(from);
            ssize_t len = recvfrom(fd, buf, sizeof(buf) - 1, 0, (struct sockaddr *)&from, &fromlen);
            if (len > 0)
            {
                buf[len] = '\0';
                printf("Received from %s:%d → %s\n", inet_ntoa(from.sin_addr), ntohs(from.sin_port), buf);
                connected = true;
            }
        }
        else
        {
            printf("no response, retrying...\n");
            struct timespec ts = {.tv_sec = 0, .tv_nsec = RETRY_INTERVAL};
            if (nanosleep(&ts, NULL) == -1)
            {
                return -1;
            }
        }
    }
    return 0;
}