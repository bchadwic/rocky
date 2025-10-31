#include "main.h"

int main(int argc, char *argv[])
{
    char *app = argv[0];
    char *s_file = NULL;
    char *r_file = NULL;

    int opt;
    while ((opt = getopt(argc, argv, "s:r:h")) != -1)
    {
        switch (opt)
        {
        case 's':
            s_file = optarg;
            break;
        case 'r':
            r_file = optarg;
            break;
        case 'h':
        default: /* '?' */
            help(app);
            return 1;
        }
    }

    bool has_send = s_file != NULL;
    bool has_recv = r_file != NULL;

    if (!(has_send || has_recv) || (has_send && has_recv))
    {
        fprintf(stderr, "%s: invalid option -- must supply exactly one of -s or -r\n", app);
        help(app);
        return 1;
    }

    if (has_send)
    {
        if (odon_send(s_file) == -1)
        {
            perror("odon_send");
            return 1;
        }
    }
    else
    {
        if (odon_recv(r_file) == -1)
        {
            perror("odon_recv");
            return 1;
        }
    }

    return 0;
}

static void help(char *app)
{
    fprintf(stderr, "Usage: %s [-s s_file] [-r r_file]\n", app);
}