#include "../include/odon.h"

static int odon_send_packet(struct odon_conn *conn, char *buf, size_t len);

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

extern int odon_send(struct odon_conn *conn, FILE *input)
{
  while (1)
  {
    printf("sending packet\n");
    char buf[PACKET_SIZE];
    size_t read_len = fread(buf, 1, PACKET_SIZE, input);
    if (read_len <= 0)
    {
      break;
    }

    if (odon_send_packet(conn, buf, read_len) < 0)
    {
      return -1;
    }
  }

  return 0;
}

static int odon_send_packet(struct odon_conn *conn, char *buf, size_t len)
{
  for (int i = 0; i < 3; i++)
  {
    if (send(conn->socket, buf, len, 0) < 0)
    {
      return -1;
    }

    struct timeval tv = {.tv_sec = 1 << i, .tv_usec = 0};
    setsockopt(conn->socket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    char ack[1];
    if (recv(conn->socket, ack, 1, 0) >= 0)
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

extern int odon_recv(struct odon_conn *conn, FILE *output)
{
  for (;;)
  {
    char buf[PACKET_SIZE];
    ssize_t read_len = recv(conn->socket, buf, PACKET_SIZE, 0);
    if (read_len <= 0)
    {
      break;
    }

    size_t written_len = fwrite(buf, 1, (size_t)read_len, output);
    if (written_len != (size_t)read_len)
    {
      return -1;
    }

    uint8_t ack[1];
    if (write(conn->socket, ack, 1) < 0)
    {
      return -1;
    }
  }

  if (fflush(output) < 0)
  {
    return -1;
  }
  return 0;
}

extern void odon_free(struct odon_conn *conn)
{
  close(conn->socket);
}