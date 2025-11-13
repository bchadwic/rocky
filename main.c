#include "./include/odon.h"
#include "./include/exch.h"
#include "./include/fmt.h"

static int run(int argc, char *argv[]);
static int send_cmd(struct sockaddr_in *src, socklen_t src_len, struct sockaddr_in *dst, socklen_t dst_len, char *filename);
static int recv_cmd(struct sockaddr_in *src, socklen_t src_len, struct sockaddr_in *dst, socklen_t dst_len, char *filename);

int main(int argc, char *argv[])
{
    struct odon_addr_exch *exch = odon_exchaddrs_init();
    for (struct odon_addr_exch *curr = exch; curr != NULL; curr = curr->next)
    {
        char encoded[MAX_EXCH_ENCODED_LENGTH];
        fmt_conn_base64url(curr->type, curr->conn_data, encoded);

        char plaintext[MAX_EXCH_PLAINTEXT_LENGTH];
        fmt_conn_plaintext(curr->type, curr->conn_data, plaintext);

        printf("%s - %s\n", encoded, plaintext);
    }
    odon_exchaddrs_free(exch);

    if (1)
    {
        return 0;
    }

    if (run(argc, argv) < 0)
    {
        if (errno != 0)
        {
            perror("run");
        }
        return 1;
    }
    return 0;
}

int run(int argc, char *argv[])
{
    if (argc < 3)
    {
        fprintf(stderr, "need 2 arguments\n");
        return -1;
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
        return send_cmd(&a1, sizeof(a1), &a2, sizeof(a2), argv[2]);
    }
    else
    {
        return recv_cmd(&a2, sizeof(a2), &a1, sizeof(a1), argv[2]);
    }
    return 0;
}

int send_cmd(struct sockaddr_in *src, socklen_t src_len, struct sockaddr_in *dst, socklen_t dst_len, char *filename)
{
    FILE *input = fopen(filename, "rb");
    if (input == NULL)
    {
        return -1;
    }

    printf("sending...\n");
    struct odon_conn conn = {0};
    if (odon_init(&conn, src, src_len, dst, dst_len) < 0)
    {
        fclose(input);
        odon_free(&conn);
        return -1;
    }

    if (odon_send(&conn, input) < 0)
    {
        fclose(input);
        odon_free(&conn);
        return -1;
    }

    fclose(input);
    odon_free(&conn);
    return 0;
}

int recv_cmd(struct sockaddr_in *src, socklen_t src_len, struct sockaddr_in *dst, socklen_t dst_len, char *filename)
{
    FILE *output = fopen(filename, "wb");
    if (output == NULL)
    {
        return -1;
    }

    printf("receiving...\n");
    struct odon_conn conn = {0};
    if (odon_init(&conn, src, src_len, dst, dst_len) < 0)
    {
        fclose(output);
        odon_free(&conn);
        return -1;
    }

    if (odon_recv(&conn, output) < 0)
    {
        fclose(output);
        odon_free(&conn);
        return -1;
    }

    fclose(output);
    odon_free(&conn);
    return 0;
}
