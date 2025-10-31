#include "send.h"

static int odon_send_read(int fd, FILE *send_file);

int odon_send(char *s_file)
{
    int fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (fd == -1)
    {
        return -1;
    }

    FILE *send_file = fopen(s_file, "r");
    if (send_file == NULL)
    {
        close(fd);
        return -1;
    }

    int res = odon_send_read(fd, send_file);
    close(fd);
    fclose(send_file);

    return res;
}

static int odon_send_read(int fd, FILE *send_file)
{
    struct sockaddr_in dst = {0};
    dst.sin_family = AF_INET;
    dst.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    dst.sin_port = htons(ODON_PORT);

    while (1)
    {
        uint8_t buf[512];
        size_t read = fread(buf, sizeof(uint8_t), 512, send_file);
        if (read == 0)
        {
            if (feof(send_file))
            {
                return 0;
            }
            return -1;
        }

        ssize_t written = sendto(fd, buf, read, 0, (struct sockaddr *)&dst, sizeof(dst));
        if (written < (ssize_t)read)
        {
            return -1;
        }
        usleep(100);
    }

    return 0;
}