#ifndef ODON_H
#define ODON_H

#define _POSIX_C_SOURCE 199309L

#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#define PACKET_SIZE 1200

struct odon_conn
{
  int socket;
};

extern int odon_init(struct odon_conn *conn,
                     struct sockaddr_in *src, socklen_t src_len,
                     struct sockaddr_in *dst, socklen_t dst_len);
extern int odon_send(struct odon_conn *conn, FILE *input);
extern int odon_recv(struct odon_conn *conn, FILE *output);
// should be called after every odon_* function that fails
extern void odon_free(struct odon_conn *conn);

#endif