#include <stdio.h>
#include <arpa/inet.h>
#include <unistd.h>
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
