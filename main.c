#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>

struct odon_conn
{
    int socket;
};

// GOAL: transmit a UDP packet reliably to a peer

extern int odon_init(
    struct odon_conn *conn,
    struct sockaddr_in *src, socklen_t src_len,
    struct sockaddr_in *dst, socklen_t dst_len);
extern int odon_send(struct odon_conn *conn, char *buf, size_t len);
/* should be called after every odon_* function that fails */
extern void odon_free(struct odon_conn *conn);

static void *await_ack(void *arg);

int main(void)
{
    struct sockaddr_in src = {0};
    src.sin_family = AF_INET;
    src.sin_addr.s_addr = INADDR_ANY;
    src.sin_port = htons(58888);

    struct sockaddr_in dst = {0};
    dst.sin_family = AF_INET;
    dst.sin_addr.s_addr = INADDR_ANY;
    dst.sin_port = htons(58887);

    struct odon_conn conn = {0};
    if (odon_init(&conn, &src, sizeof(src), &dst, sizeof(dst)) < 0)
    {
        odon_free(&conn);
        return 1;
    }

    char *buf = "hello world\n";
    size_t len = strlen(buf);

    if (odon_send(&conn, buf, len) < 0)
    {
        odon_free(&conn);
        return 1;
    }

    odon_free(&conn);
    return 0;
}

extern int odon_init(
    struct odon_conn *conn,
    struct sockaddr_in *src, socklen_t src_len,
    struct sockaddr_in *dst, socklen_t dst_len)
{
    conn->socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
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
    pthread_t verify;
    pthread_create(&verify, NULL, await_ack, (void *)conn);

    if (send(conn->socket, buf, len, 0) < 0)
    {
        pthread_cancel(verify);
        return -1;
    }

    pthread_join(verify, NULL);
    return 0;
}

extern void odon_free(struct odon_conn *conn)
{
    close(conn->socket);
}

static void *await_ack(void *arg)
{
    struct odon_conn *conn = (struct odon_conn *)arg;

    // listen for ack

    return NULL;
}
