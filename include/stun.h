#ifndef STUN_H
#define STUN_H

#include <sys/types.h>

#define ROCKY_PORT 60357
#define STUN_PORT 19302

#define STUN_HEADER_SIZE 20
#define STUN_REQUEST_SIZE 20
#define STUN_RESPONSE_SIZE 512

#define STUN_REQUEST 0x0001
#define STUN_RESPONSE 0x0101

#define STUN_HOSTNAME "stun.l.google.com"
#define STUN_MAGIC_COOKIE 0x2112A442
#define STUN_ATTR_XOR_MAPPED_ADDRESS 0x0020

typedef struct
{
  // req
  uint16_t type;
  uint16_t len;
  uint8_t transaction_identifier[12];

  // resp
  uint16_t port;
  uint32_t ip_addr;
} stun_message_t;

/*
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|0 0|     STUN Message Type     |         Message Length        |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                         Magic Cookie                          |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
|                     Transaction ID (96 bits)                  |
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/
int stun(int fd, stun_message_t *msg);

#endif