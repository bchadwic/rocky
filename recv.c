#include "recv.h"

static int odon_socket(void);
static int odon_write_recvd(int fd, FILE *recv_file);

int odon_recv(char *r_file)
{
    int fd = odon_socket();
    if (fd == -1)
    {
        return -1;
    }

    FILE *recv_file = fopen(r_file, "a");
    if (recv_file == NULL)
    {
        close(fd);
        return -1;
    }

    int res = odon_write_recvd(fd, recv_file);
    close(fd);
    fclose(recv_file);

    return res;
}

static int odon_socket(void)
{
    int fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (fd == -1)
    {
        return -1;
    }

    struct sockaddr_in local = {0};
    local.sin_family = AF_INET;
    local.sin_addr.s_addr = INADDR_ANY;
    local.sin_port = htons(ODON_PORT);
    if (bind(fd, (struct sockaddr *)&local, sizeof(local)) == -1)
    {

        close(fd);
        return -1;
    }

    int size = 4 * 1024 * 1024; // 4 MB
    if (setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size)) == -1)
    {
        close(fd);
        return -1;
    }

    return fd;
}

static int odon_write_recvd(int fd, FILE *recv_file)
{
    while (1)
    {
        uint8_t buf[512];
        ssize_t n = recv(fd, buf, sizeof(buf), 0);
        if (n == -1)
        {
            return -1;
        }

        size_t read = (size_t)n;
        size_t written = fwrite(buf, sizeof(uint8_t), read, recv_file);
        if (written < read)
        {
            return -1;
        }

        if (fflush(recv_file) != 0)
        {
            return -1;
        }
    }
    return 0;
}
