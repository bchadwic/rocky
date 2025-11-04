#define _POSIX_C_SOURCE 199309L

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

struct odon_conn
{
    int socket;
    int ack_received;
    pthread_cond_t ack_cond;
    pthread_mutex_t mutex;
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
    src.sin_addr.s_addr = htonl(INADDR_ANY);
    src.sin_port = htons(58888);

    struct sockaddr_in dst = {0};
    dst.sin_family = AF_INET;
    dst.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    dst.sin_port = htons(58887);

    struct odon_conn conn = {0};
    if (odon_init(&conn, &src, sizeof(src), &dst, sizeof(dst)) < 0)
    {
        odon_free(&conn);
        perror("odon_init");
        return 1;
    }

    char *buf = "hello world\n";
    size_t len = strlen(buf);

    if (odon_send(&conn, buf, len) < 0)
    {
        odon_free(&conn);
        perror("odon_send");
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

    conn->ack_received = 0;
    pthread_mutex_init(&conn->mutex, NULL);
    pthread_cond_init(&conn->ack_cond, NULL);
    return 0;
}

extern int odon_send(struct odon_conn *conn, char *buf, size_t len)
{
    pthread_t tid;
    pthread_create(&tid, NULL, await_ack, (void *)conn);

    int timedwait_sec = 1; // 1s
    int acked = 0;
    while (!acked)
    {
        if (send(conn->socket, buf, len, 0) < 0)
        {
            pthread_cancel(tid);
            return -1;
        }

        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += timedwait_sec;

        pthread_mutex_lock(&conn->mutex);
        while (!conn->ack_received)
        {
            int res = pthread_cond_timedwait(&conn->ack_cond, &conn->mutex, &ts);
            if (res == ETIMEDOUT)
            {
                printf("ack timed out\n");
                timedwait_sec *= 2;
                break;
            }
            else if (res < 0)
            {
                pthread_mutex_unlock(&conn->mutex);
                pthread_cancel(tid);
                return -1;
            }
        }
        acked = conn->ack_received;
        pthread_mutex_unlock(&conn->mutex);
    }

    pthread_join(tid, NULL);
    return 0;
}

extern void odon_free(struct odon_conn *conn)
{
    close(conn->socket);
    pthread_mutex_destroy(&conn->mutex);
    pthread_cond_destroy(&conn->ack_cond);
}

static void *await_ack(void *arg)
{
    struct odon_conn *conn = (struct odon_conn *)arg;

    int status = 1;
    uint8_t buf[512] = {0};
    read(conn->socket, buf, 512); // handle error

    pthread_mutex_lock(&conn->mutex);
    conn->ack_received = status;
    pthread_cond_signal(&conn->ack_cond);
    pthread_mutex_unlock(&conn->mutex);
    return NULL;
}
