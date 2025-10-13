#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/random.h>
#include <netdb.h>
#include <sys/time.h>
#include <stdbool.h>
#include <sys/time.h>
#include "../include/stun.h"
#include "../include/peer.h"

int rocky(int fd)
{
    struct sockaddr_in local = {0};
    local.sin_family = AF_INET;
    local.sin_addr.s_addr = INADDR_ANY;
    local.sin_port = htons(ROCKY_PORT);

    if (bind(fd, (struct sockaddr *)&local, sizeof(local)) == -1)
    {
        return -1;
    }

    struct sockaddr_in public_addr = {0};
    if (getpublicaddress(fd, &public_addr) == -1)
    {
        return -1;
    }
    printf("public: %s:%d\n", inet_ntoa(public_addr.sin_addr), public_addr.sin_port);

    struct sockaddr_in peer_addr = {0};
    if (getpeeraddress(&peer_addr) == -1)
    {
        return -1;
    }

    fd_set readfds;
    char buf[1024];

    bool connected = false;
    while (!connected)
    {
        char *ping = "ping";
        sendto(fd, ping, sizeof(ping), 0, (struct sockaddr *)&peer_addr, sizeof(peer_addr));
        printf("ping sent\n");

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
            sleep(1);
        }
    }
    return 0;
}

int main(void)
{
    int fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (fd == -1)
    {
        return -1;
    }

    int res = rocky(fd);
    close(fd);

    if (res == -1)
    {
        perror("failed");
        return 1;
    }
    return 0;
}
