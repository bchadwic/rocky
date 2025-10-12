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

int getpublicaddress(stun_message_t *msg)
{
    int fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (fd == -1)
    {
        return 1;
    }

    struct sockaddr_in local = {0};
    local.sin_family = AF_INET;
    local.sin_addr.s_addr = INADDR_ANY;
    local.sin_port = htons(ROCKY_PORT);

    if (bind(fd, (struct sockaddr *)&local, sizeof(local)) == -1)
    {
        close(fd);
        return 1;
    }

    int res = stun(fd, msg);
    close(fd);
    return res;
}

int main(void)
{
    stun_message_t msg;
    if (getpublicaddress(&msg) == -1)
    {
        perror("getpublicaddress");
        return 1;
    }

    struct in_addr addr;
    addr.s_addr = htonl(msg.ip_addr);
    uint32_t port = htons(msg.port);
    printf("addr: %s, port: %d\n", inet_ntoa(addr), port);
}