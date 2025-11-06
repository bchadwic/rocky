#define _POSIX_C_SOURCE 199309L

#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

struct odon_conn
{
    int socket;
};

extern int odon_init(struct odon_conn *conn,
                     struct sockaddr_in *src, socklen_t src_len,
                     struct sockaddr_in *dst, socklen_t dst_len);
extern int odon_send(struct odon_conn *conn, char *buf, size_t len);
extern int odon_recv(struct odon_conn *conn, char *buf, size_t len);
// should be called after every odon_* function that fails
extern void odon_free(struct odon_conn *conn);

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("need 1 arguments\n");
        return 1;
    }

    if (strcmp(argv[1], "send") == 0)
    {
        printf("sending...\n");
        char *buf = "hello world";
        size_t len = strlen(buf);

        struct sockaddr_in src = {0};
        src.sin_family = AF_INET;
        src.sin_addr.s_addr = htonl(INADDR_ANY);
        src.sin_port = htons(52888);

        struct sockaddr_in dst = {0};
        dst.sin_family = AF_INET;
        dst.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        dst.sin_port = htons(52887);

        struct odon_conn conn = {0};
        if (odon_init(&conn, &src, sizeof(src), &dst, sizeof(dst)) < 0)
        {
            odon_free(&conn);
            perror("odon_init");
            return 1;
        }

        if (odon_send(&conn, buf, len) < 0)
        {
            odon_free(&conn);
            perror("odon_send");
            return 1;
        }
        odon_free(&conn);
    }
    else
    {
        printf("receiving...\n");

        struct sockaddr_in src = {0};
        src.sin_family = AF_INET;
        src.sin_addr.s_addr = htonl(INADDR_ANY);
        src.sin_port = htons(52887);

        struct sockaddr_in dst = {0};
        dst.sin_family = AF_INET;
        dst.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        dst.sin_port = htons(52888);

        struct odon_conn conn = {0};
        if (odon_init(&conn, &src, sizeof(src), &dst, sizeof(dst)) < 0)
        {
            odon_free(&conn);
            perror("odon_init");
            return 1;
        }
        char buf[512] = {0};
        if (odon_recv(&conn, buf, 512) < 0)
        {
            odon_free(&conn);
            perror("odon_recv");
            return -1;
        }
        printf("recv: %s\n", buf);
        odon_free(&conn);
    }

    return 0;
}

extern int odon_init(
    struct odon_conn *conn,
    struct sockaddr_in *src, socklen_t src_len,
    struct sockaddr_in *dst, socklen_t dst_len)
{
    conn->socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (conn->socket < 0)
    {
        return -1;
    }

    if (bind(conn->socket, (struct sockaddr *)src, src_len) < 0)
    {
        return -1;
    }

    if (connect(conn->socket, (struct sockaddr *)dst, dst_len) < 0)
    {
        return -1;
    }
    return 0;
}

// extern int odon_send_packet(struct odon_conn *conn, char *buf, size_t len)
// {
// }

extern int odon_send(struct odon_conn *conn, char *buf, size_t len)
{
    for (int i = 1; i <= 3; i++)
    {
        if (send(conn->socket, buf, len, 0) < 0)
        {
            return -1;
        }

        struct timeval tv = {.tv_sec = 1 * i, .tv_usec = 0};
        setsockopt(conn->socket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

        char ack[512];
        if (recv(conn->socket, ack, 512, 0) >= 0)
        {
            break;
        }
        else if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            continue;
        }

        return -1;
    }

    return 0;
}

extern int odon_recv(struct odon_conn *conn, char *buf, size_t len)
{
    if (read(conn->socket, buf, len) < 0)
    {
        return -1;
    }

    uint8_t *ack[512];
    if (write(conn->socket, ack, 512) < 0)
    {
        return -1;
    }

    return 0;
}

extern void odon_free(struct odon_conn *conn)
{
    close(conn->socket);
}