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

int stun(uint8_t *buf);

int main(void)
{
    int fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (fd == -1)
    {
        perror("socket");
        return 1;
    }

    // create stun packet
    uint8_t buf[20] = {0};
    if (stun(buf) == -1)
    {
        close(fd);
        fprintf(stderr, "stun: Could not create a request");
        return 1;
    }

    struct hostent *host = gethostbyname("stun.l.google.com");
    char **addr_list = host->h_addr_list;

    // retry logic for multiple ip addresses
    uint8_t sent = 0;
    while (!sent && *addr_list != NULL)
    {
        struct in_addr addr;
        memcpy(&addr, *addr_list, sizeof(struct in_addr));

        struct sockaddr_in dst = {0};
        dst.sin_family = AF_INET;
        dst.sin_addr.s_addr = addr.s_addr;
        dst.sin_port = htons(19302);

        if (sendto(fd, buf, sizeof(buf), 0, (struct sockaddr *)&dst, sizeof(dst)) != -1)
        {
            sent = 1;
        }

        addr_list++;
    }

    if (!sent)
    {
        perror("sendto");
        close(fd);
        return 1;
    }

    // timeout
    struct timeval tv = {.tv_sec = 2, .tv_usec = 0};
    if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) == -1)
    {
        perror("setsockopt");
        close(fd);
        return 1;
    }

    // await reply
    uint8_t reply[512] = {0};
    struct sockaddr_in src = {0};
    socklen_t srclen = sizeof(src);
    ssize_t received = recvfrom(fd, reply, sizeof(reply), 0, (struct sockaddr *)&src, &srclen);
    if (received == -1)
    {
        perror("recvfrom");
        close(fd);
        return 1;
    }

    // discard first byte
    uint8_t family = reply[1];
    if (family == 0x01)
    {
        printf("ipv4\n");
    }
    else
    {
        // don't support ipv6 yet
        fprintf(stderr, "unsupported ipv6\n");
        close(fd);
        return 1;
    }

    uint16_t message_type = (reply[0] << 8) | reply[1];
    if (message_type != 0x0101)
    {
        fprintf(stderr, "response was not successful, need to handle error\n");
        close(fd);
        return 1;
    }

    for (int i = 0; i < received; i++)
    {
        if (i % 4 == 0)
            printf("\n");
        printf("%#04x   ", reply[i]);
    }
    printf("\n");

    uint32_t magic_cookie = 0x2112A442;
    uint32_t reply_cookie;
    memcpy(&reply_cookie, reply + 4, sizeof(magic_cookie));

    if (ntohl(reply_cookie) != magic_cookie)
    {

        fprintf(stderr, "reply cookie did not match magic cookie\n");
        close(fd);
        return 1;
    }

    close(fd);
    return 0;
}

/*

    printf("message type: %04X\n", message_type);

    uint16_t p = reply[2] << 8 | reply[3];
    printf("port: %04X\n", p);

    uint8_t port[2] = {reply[32 - 6], reply[31 - 5]};
    uint8_t magic_cookie_bytes[4] = {0x21, 0x12, 0xA4, 0x42};
    uint16_t decoded_port = ((port[0] ^ magic_cookie_bytes[0]) << 8) | (port[1] ^ magic_cookie_bytes[1]);

    printf("port: %02X, %02X\n", port[0], port[1]);
    printf("port: %04X\n", decoded_port);

*/
// uint8_t port[2] = { reply[32 - 6], reply[31 - 5] };
// uint8_t address[4] = { reply[32 - 4], reply[32 - 3], reply[32 - 2], reply[32 - 1] };

// uint8_t magic_cookie_bytes[4] = { 0x21, 0x12, 0xA4, 0x42 };
// uint16_t decoded_port = ((port[0] ^ magic_cookie_bytes[0]) << 8) | (port[1] ^ magic_cookie_bytes[1]);

// uint8_t decoded_addr[4];
// for (int i = 0; i < 4; i++) {
//     decoded_addr[i] = address[i] ^ magic_cookie_bytes[i];
// }

// char ip_str[INET_ADDRSTRLEN];
// sprintf(ip_str, "%u.%u.%u.%u", decoded_addr[0], decoded_addr[1], decoded_addr[2], decoded_addr[3]);

// printf("Public IP: %s, Port: %u\n", ip_str, decoded_port);

/*
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|0 0|     STUN Message Type     |         Message Length        |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                         Magic Cookie                          |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
|                     Transaction ID (96 bits)                  |
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/
int stun(uint8_t *buf)
{
    buf[0] = buf[1] = 0;

    // request
    uint16_t message_type = htons(0x0001);
    uint16_t message_length = htons(0x0000);
    uint32_t magic_cookie = htonl(0x2112A442);

    uint8_t transaction_identifier[12];
    if (getrandom(transaction_identifier, sizeof(transaction_identifier), 0) != sizeof(transaction_identifier))
    {
        return -1;
    }

    memcpy(buf, &message_type, sizeof(message_type));
    memcpy(buf + 2, &message_length, sizeof(message_length));
    memcpy(buf + 4, &magic_cookie, sizeof(magic_cookie));
    memcpy(buf + 8, &transaction_identifier, sizeof(transaction_identifier));
    return 0;
}