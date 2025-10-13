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
#include "../include/stun.h"

int rocky(int fd)
{
    struct sockaddr_in local = {0};
    local.sin_family = AF_INET;
    local.sin_addr.s_addr = INADDR_ANY;
    local.sin_port = htons(ROCKY_PORT);

    // if (bind(fd, (struct sockaddr *)&local, sizeof(local)) == -1)
    // {
    //     return -1;
    // }

    // stun_message_t msg;
    // if (getpublicaddress(fd, &msg) == -1)
    // {
    //     return -1;
    // }

    // struct in_addr host_addr;
    // host_addr.s_addr = htonl(msg.ip_addr);
    // uint32_t host_port = htons(msg.port);
    // printf("host %s:%d\n", inet_ntoa(host_addr), host_port);

    printf("host: 174.61.173.64:17390\n");

    char peer[100];
    printf("peer: ");
    scanf("%s", peer);

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
        perror("run");
        return 1;
    }
    return 0;
}