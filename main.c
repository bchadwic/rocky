#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>

struct odon_conn
{
    int fd;
};

// GOAL: transmit a UDP packet reliably to a peer

extern int odon_init(struct odon_conn *conn, in_addr_t ip, in_port_t port);
extern int odon_send(struct odon_conn *conn, char *buf, size_t len);
extern void odon_free(struct odon_conn *conn);

static void *await_ack(void *arg);

int main(void)
{
    struct odon_conn conn = {0};
    if (odon_init(&conn, INADDR_ANY, htons(58888)) < 0)
    {
        return 1;
    }

    char *buf = "hello world\n";
    size_t len = sizeof(buf);

    if (odon_send(&conn, buf, len) < 0)
    {
        return 1;
    }

    odon_free(&conn);
    return 0;
}

int odon_conn(struct odon_conn *conn, in_addr_t ip, in_port_t port)
{
    int fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (fd < 0)
    {
        return -1;
    }

    struct sockaddr_in local = {0};
    local.sin_family = AF_INET;
    local.sin_addr.s_addr = ip;
    local.sin_port = port;

    if (bind(fd, (struct sockaddr *)&local, sizeof(local)) < 0)
    {
        return -1;
    }

    conn->fd = fd;
    return 0;
}

int odon_send(struct odon_conn *conn, char *buf, size_t len)
{
    pthread_t verify;
    pthread_create(&verify, NULL, await_ack, (void *)conn);

    // send UDP packet

    pthread_join(verify, NULL);
    return 0;
}

extern void odon_free(struct odon_conn *conn)
{
    close(conn->fd);
}

static void *await_ack(void *arg)
{
    struct odon_conn *conn = (struct odon_conn *)arg;

    // listen for ack

    return NULL;
}
