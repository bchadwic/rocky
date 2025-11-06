#include "../include/odon.h"

extern int odon_init(struct odon_conn *conn,
                     struct sockaddr_in *src, socklen_t src_len,
                     struct sockaddr_in *dst, socklen_t dst_len)
{
  conn->socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (conn->socket < 0)
  {
    return -1;
  }

  if (bind(conn->socket, (struct sockaddr *)src, src_len) < 0)
  {
    return -1;
  }

  if (connect(conn->socket, (struct sockaddr *)dst, dst_len) < 0)
  {
    return -1;
  }
  return 0;
}

extern int odon_send(struct odon_conn *conn, char *buf, size_t len)
{
  for (int i = 0; i < 3; i++)
  {
    if (send(conn->socket, buf, len, 0) < 0)
    {
      return -1;
    }

    struct timeval tv = {.tv_sec = 1 << i, .tv_usec = 0};
    setsockopt(conn->socket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    char ack[512];
    if (recv(conn->socket, ack, 512, 0) >= 0)
    {
      break;
    }
    else if (errno == EAGAIN || errno == EWOULDBLOCK)
    {
      continue;
    }

    return -1;
  }

  return 0;
}

extern int odon_recv(struct odon_conn *conn, char *buf, size_t len)
{
  if (read(conn->socket, buf, len) < 0)
  {
    return -1;
  }

  uint8_t ack[512];
  if (write(conn->socket, ack, 512) < 0)
  {
    return -1;
  }

  return 0;
}

extern void odon_free(struct odon_conn *conn)
{
  close(conn->socket);
}