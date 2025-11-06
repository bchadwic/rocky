#include "./include/odon.h"

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
