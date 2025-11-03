#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>

// GOAL: transmit a UDP packet reliably to a peer

extern int odon_socket(struct sockaddr_in *local, socklen_t len);
extern int odon_send(int fd, char *buf, size_t len);

static void *await_ack(void *arg);

int main(void)
{
    struct sockaddr_in local = {0};
    local.sin_family = AF_INET;
    local.sin_addr.s_addr = INADDR_ANY;
    local.sin_port = htons(58888);

    int fd = odon_socket(&local, sizeof(local));
    if (fd < 0)
    {
        return 1;
    }

    char *buf = "hello world\n";
    size_t len = sizeof(buf);

    if (odon_send(fd, buf, len) < 0)
    {
        return 1;
    }
    close(fd);
    return 0;
}

int odon_socket(struct sockaddr_in *local, socklen_t len)
{
    int fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (fd < 0)
    {
        return -1;
    }

    if (bind(fd, (struct sockaddr *)local, len) < 0)
    {
        return -1;
    }

    return fd;
}

int odon_send(int fd, char *buf, size_t len)
{
    pthread_t verify;
    pthread_create(&verify, NULL, await_ack, fd);

    // send UDP packet

    pthread_join(verify, NULL);
    return 0;
}

static void *await_ack(void *arg)
{
    int fd = (int)arg;

    // listen for ack

    return NULL;
}
