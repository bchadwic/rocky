#include "./include/odon.h"

static int run(int argc, char *argv[]);
static int send_cmd(struct sockaddr_in *src, socklen_t src_len, struct sockaddr_in *dst, socklen_t dst_len, char *filename);
static int recv_cmd(struct sockaddr_in *src, socklen_t src_len, struct sockaddr_in *dst, socklen_t dst_len);

int main(int argc, char *argv[])
{
    if (run(argc, argv) < 0)
    {
        perror("run");
        return 1;
    }
    return 0;
}

int run(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("need 1 argument\n");
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
        if (argc < 3)
        {
            printf("send needs 2 arguments\n");
            return 1;
        }
        return send_cmd(&a1, sizeof(a1), &a2, sizeof(a2), argv[2]);
    }
    else
    {
        return recv_cmd(&a2, sizeof(a2), &a1, sizeof(a1));
    }
    return 0;
}

int send_cmd(struct sockaddr_in *src, socklen_t src_len, struct sockaddr_in *dst, socklen_t dst_len, char *filename)
{
    FILE *fp = fopen(filename, "rb");
    if (fp == NULL)
    {
        return -1;
    }

    char buf[4096];
    size_t n = fread(buf, 1, sizeof(buf), fp);
    if (n == 0)
    {
        fclose(fp);
        return -1;
    }

    printf("sending...\n");
    struct odon_conn conn = {0};
    if (odon_init(&conn, src, src_len, dst, dst_len) < 0)
    {
        fclose(fp);
        odon_free(&conn);
        return -1;
    }

    if (odon_send(&conn, buf, n) < 0)
    {
        fclose(fp);
        odon_free(&conn);
        return -1;
    }

    fclose(fp);
    odon_free(&conn);
    return 0;
}

int recv_cmd(struct sockaddr_in *src, socklen_t src_len, struct sockaddr_in *dst, socklen_t dst_len)
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

    odon_free(&conn);
    return 0;
}
