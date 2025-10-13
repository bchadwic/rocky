#include <stdio.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>

int getpeeraddress(struct sockaddr_in *addr)
{
    printf("peer: ");
    char in[32];
    if (!fgets(in, sizeof(in), stdin))
    {
        return -1;
    }
    in[strcspn(in, "\n")] = 0;

    char ip[16];
    int port;
    if (sscanf(in, "%15[^:]:%d", ip, &port) != 1)
    {
        return -1;
    }

    if (port < 1 || port > 65535)
    {
        return -1;
    }

    addr->sin_family = AF_INET;
    addr->sin_port = htons(port);
    if (inet_pton(AF_INET, ip, &addr->sin_addr.s_addr) != 1)
    {
        return -1;
    }
    return 0;
}