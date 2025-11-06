#include "./include/odon.h"

int run(int argc, char *argv[]);
int send_cmd(struct sockaddr_in *src,
             socklen_t src_len,
             struct sockaddr_in *dst,
             socklen_t dst_len);
int recv_cmd(struct sockaddr_in *src,
             socklen_t src_len,
             struct sockaddr_in *dst,
             socklen_t dst_len);

int send_cmd(struct sockaddr_in *src,
             socklen_t src_len,
             struct sockaddr_in *dst,
             socklen_t dst_len)
{

    printf("sending...\n");
    char *buf = "hello world";
    size_t len = strlen(buf);

    struct odon_conn conn = {0};
    if (odon_init(&conn, src, src_len, dst, dst_len) < 0)
    {
        odon_free(&conn);
        return -1;
    }

    if (odon_send(&conn, buf, len) < 0)
    {
        odon_free(&conn);
        return -1;
    }

    odon_free(&conn);
    return 0;
}

int recv_cmd(struct sockaddr_in *src,
             socklen_t src_len,
             struct sockaddr_in *dst,
             socklen_t dst_len)
{
    printf("receiving...\n");
    struct odon_conn conn = {0};
    if (odon_init(&conn, src, src_len, dst, dst_len) < 0)
    {
        odon_free(&conn);
        return -1;
    }
    char buf[512] = {0};
    if (odon_recv(&conn, buf, 512) < 0)
    {
        odon_free(&conn);
        return -1;
    }

    printf("recv: %s\n", buf);
    odon_free(&conn);
    return 0;
}

int run(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("need 1 arguments\n");
        return 1;
    }

    struct sockaddr_in a1 = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = htonl(INADDR_LOOPBACK),
        .sin_port = htons(52888),
    };

    struct sockaddr_in a2 = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = htonl(INADDR_LOOPBACK),
        .sin_port = htons(52887),
    };

    if (strcmp(argv[1], "send") == 0)
    {
        return send_cmd(&a1, sizeof(a1), &a2, sizeof(a2));
    }
    else
    {
        return recv_cmd(&a2, sizeof(a2), &a1, sizeof(a1));
    }
}

int main(int argc, char *argv[])
{
    if (run(argc, argv) < 0)
    {
        perror("run");
        return 1;
    }
    return 0;
}