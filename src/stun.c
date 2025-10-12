#include <errno.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/random.h>
#include <netdb.h>
#include <sys/time.h>
#include <stdbool.h>
#include <stdint.h>
#include "../include/stun.h"

static int stununpack(uint8_t *resp, ssize_t n, stun_message_t *msg)
{
  if (!resp || !msg || n < 20)
    return -1; // too short

  // STUN header (big-endian)
  msg->type = (resp[0] << 8) | resp[1];
  msg->len = (resp[2] << 8) | resp[3];

  uint32_t magic = (resp[4] << 24) | (resp[5] << 16) | (resp[6] << 8) | resp[7];
  if (magic != STUN_MAGIC_COOKIE)
    return -1;

  // Copy transaction ID
  for (int i = 0; i < 12; i++)
    msg->transaction_identifier[i] = resp[8 + i];

  // Parse attributes
  ssize_t offset = 20;
  while (offset + 4 <= n)
  {
    uint16_t attr_type = (resp[offset] << 8) | resp[offset + 1];
    uint16_t attr_len = (resp[offset + 2] << 8) | resp[offset + 3];
    offset += 4;

    if (offset + attr_len > n)
      return -1; // malformed attribute

    if (attr_type == STUN_ATTR_XOR_MAPPED_ADDRESS && attr_len >= 8)
    {
      // Assume IPv4
      uint8_t family = resp[offset + 1];
      if (family != 0x01)
        return -1;

      // Decode port
      uint16_t xport = (resp[offset + 2] << 8) | resp[offset + 3];
      xport ^= (STUN_MAGIC_COOKIE >> 16) & 0xFFFF;
      msg->port = xport;

      // Decode IP
      uint32_t xaddr =
          ((uint32_t)resp[offset + 4] << 24) |
          ((uint32_t)resp[offset + 5] << 16) |
          ((uint32_t)resp[offset + 6] << 8) |
          ((uint32_t)resp[offset + 7]);
      xaddr ^= STUN_MAGIC_COOKIE;
      msg->ip_addr = xaddr;

      return 0; // success
    }

    // Move to next attribute (pad to 4-byte boundary)
    offset += ((attr_len + 3) / 4) * 4;
  }

  return -1; // XOR-MAPPED-ADDRESS not found
}

static int stunrecv(int fd, uint8_t *resp, size_t n)
{
  struct timeval tv = {.tv_sec = 2, .tv_usec = 0};
  if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) == -1)
  {
    return -1;
  }

  struct sockaddr_in src = {0};
  socklen_t src_len = sizeof(src);
  ssize_t recvd = recvfrom(fd, resp, n, 0, (struct sockaddr *)&src, &src_len);
  if (recvd == -1)
  {
    return -1;
  }
  return recvd;
}

static int stunsend(int fd, uint8_t *req, size_t n, const char *hostname)
{
  struct hostent *host = gethostbyname(hostname);
  char **addr_list = host->h_addr_list;
  if (host == NULL || host->h_addr_list == NULL)
  {
    return -1;
  }

  // retry logic for multiple ip addresses
  bool sent = false;
  while (!sent && *addr_list != NULL)
  {
    struct in_addr addr;
    memcpy(&addr, *addr_list, sizeof(struct in_addr));

    struct sockaddr_in dst = {0};
    dst.sin_family = AF_INET;
    dst.sin_addr = addr;
    dst.sin_port = htons(STUN_PORT);

    if (sendto(fd, req, n, 0, (struct sockaddr *)&dst, sizeof(dst)) != -1)
    {
      sent = true;
    }
    addr_list++;
  }

  if (!sent)
  {
    return -1;
  }
  return 0;
}

static int stunpack(uint8_t *buf, size_t n, const stun_message_t *msg)
{
  if (n < STUN_REQUEST_SIZE)
  {
    errno = EINVAL;
    return -1;
  }
  memset(buf, 0, n);

  uint16_t type = htons(msg->type);
  uint16_t len = htons(msg->len);
  memcpy(buf, &type, sizeof(type));
  memcpy(buf + 2, &len, sizeof(len));

  uint32_t magic_cookie = htonl(STUN_MAGIC_COOKIE);
  memcpy(buf + 4, &magic_cookie, sizeof(magic_cookie));
  memcpy(buf + 8, &msg->transaction_identifier, sizeof(msg->transaction_identifier));
  return 0;
}

static int stuninit(stun_message_t *msg)
{
  msg->type = STUN_REQUEST;
  msg->len = 0x0000;
  return getrandom(msg->transaction_identifier, sizeof(msg->transaction_identifier), 0);
}

int stun(int fd, stun_message_t *msg)
{
  if (stuninit(msg) == -1)
  {
    return -1;
  }

  uint8_t req[STUN_REQUEST_SIZE] = {0};
  if (stunpack(req, sizeof(req), msg) == -1)
  {
    return -1;
  }

  if (stunsend(fd, req, sizeof(req), STUN_HOSTNAME) == -1)
  {
    return -1;
  }

  uint8_t resp[STUN_RESPONSE_SIZE] = {0};
  ssize_t n = stunrecv(fd, resp, sizeof(resp));
  if (n == -1)
  {
    return -1;
  }

  if (stununpack(resp, n, msg) == -1)
  {
    return -1;
  }

  return 0;
}