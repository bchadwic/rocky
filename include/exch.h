#ifndef CONN_H
#define CONN_H

#define _GNU_SOURCE

#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <net/if.h>

#define MAX_EXCH_DATA_LENGTH 6

enum exch_type
{
  IPV4_LOCAL_AREA = 4,
  IPV4_REFLEXIVE = 6
};

struct odon_addr_exch
{
  enum exch_type type;
  uint8_t conn_data[MAX_EXCH_DATA_LENGTH];
  struct odon_addr_exch *next;
};

extern struct odon_addr_exch *odon_exchaddrs(void);

#endif