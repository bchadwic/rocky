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
    if (argc < 4)
    {
        printf("need 3 arguments\n");
        return 1;
    }

    char *arg = argv[1];
    int src_port = atoi(argv[2]);
    int dst_port = atoi(argv[3]);
    if (src_port < 0 || dst_port < 0)
    {
        printf("arguments must be numeric\n");
        return 1;
    }

    struct sockaddr_in src = {0};
    src.sin_family = AF_INET;
    src.sin_addr.s_addr = htonl(INADDR_ANY);
    src.sin_port = htons(src_port);

    struct sockaddr_in dst = {0};
    dst.sin_family = AF_INET;
    dst.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    dst.sin_port = htons(dst_port);

    struct odon_conn conn = {0};
    if (odon_init(&conn, &src, sizeof(src), &dst, sizeof(dst)) < 0)
    {
        odon_free(&conn);
        perror("odon_init");
        return 1;
    }

    if (strcmp(arg, "send") == 0)
    {
        char *buf = "hello world";
        size_t len = strlen(buf);

        if (odon_send(&conn, buf, len) < 0)
        {
            odon_free(&conn);
            perror("odon_send");
            return 1;
        }
    }
    else
    {
        char buf[512] = {0};
        if (odon_recv(&conn, buf, 512) < 0)
        {
            odon_free(&conn);
            perror("odon_recv");
            return -1;
        }
        printf("recv: %s\n", buf);
    }

    odon_free(&conn);
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

extern int odon_send(struct odon_conn *conn, char *buf, size_t len)
{

    int success = 0;
    for (int i = 1; i <= 3; i++)
    {
        if (send(conn->socket, buf, len, 0) < 0)
        {
            return -1;
        }

        struct timeval tv = {.tv_sec = 1 * i, .tv_usec = 0};
        setsockopt(conn->socket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

        char ack[512];
        int n = recv(conn->socket, ack, 512, 0);
        if (n >= 0)
        {
            success = 1;
            break;
        }

        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            continue;
        }
        return -1;
    }

    if (!success)
    {
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