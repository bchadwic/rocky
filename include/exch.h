#ifndef CONN_H
#define CONN_H

#define _GNU_SOURCE

#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <net/if.h>
#include <stdlib.h>

#define MAX_EXCH_DATA_LENGTH 6

enum exch_type
{
  IPV4_LOCAL_AREA = 4,
  IPV4_REFLEXIVE = 6
};

struct odon_addr_exch
{
  struct odon_addr_exch *next;

  enum exch_type type;
  uint8_t conn_data[MAX_EXCH_DATA_LENGTH];
};

extern struct odon_addr_exch *odon_exchaddrs_init(void);
// should be called after using odon_addr_exch
extern void odon_exchaddrs_free(struct odon_addr_exch *exch);

#endif