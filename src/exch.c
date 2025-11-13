#include "../include/exch.h"

struct odon_addr_exch *odon_exchaddrs_init(void)
{
  struct odon_addr_exch *exch = NULL;
  struct odon_addr_exch **tail = &exch;

  struct ifaddrs *ifaddr = NULL;
  if (getifaddrs(&ifaddr) == -1)
  {
    return NULL;
  }

  for (struct ifaddrs *ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
  {
    // if the address is not ipv4, is not up or is a loopback, skip
    if (ifa->ifa_addr == NULL ||
        ifa->ifa_addr->sa_family != AF_INET ||
        !(ifa->ifa_flags & IFF_UP) ||
        ifa->ifa_flags & IFF_LOOPBACK)
    {
      continue;
    }

    struct sockaddr_in *addr = (struct sockaddr_in *)ifa->ifa_addr;
    struct odon_addr_exch *ex = malloc(sizeof(struct odon_addr_exch));
    if (ex == NULL)
    {
      freeifaddrs(ifaddr);
      return NULL;
    }

    ex->next = NULL;
    ex->type = IPV4_LOCAL_AREA;
    memcpy(ex->conn_data, &addr->sin_addr.s_addr, IPV4_LOCAL_AREA);

    *tail = ex;
    tail = &ex->next;
  }

  freeifaddrs(ifaddr);
  return exch;
}

extern void odon_exchaddrs_free(struct odon_addr_exch *exch)
{
  while (exch)
  {
    struct odon_addr_exch *next = exch->next;
    free(exch);
    exch = next;
  }
  return;
}