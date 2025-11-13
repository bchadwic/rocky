#include "../include/exch.h"

struct odon_addr_exch *odon_exchaddrs(void)
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
    if (ifa->ifa_addr == NULL)
    {
      continue;
    }

    int family = ifa->ifa_addr->sa_family;
    if (family != AF_INET || !(ifa->ifa_flags & IFF_UP) || ifa->ifa_flags & IFF_LOOPBACK)
    {
      continue;
    }

    struct sockaddr_in *ip = (struct sockaddr_in *)ifa->ifa_addr;

    // inet_ntop(AF_INET, (void *)&ip->sin_addr, addr, sizeof(addr));

    // printf("%s: %s\n", ifa->ifa_name, addr);
    struct odon_addr_exch *ex = malloc(sizeof(struct odon_addr_exch));
    memcpy(ex->conn_data, ip->sin_addr.s_addr, 4);
  }
  freeifaddrs(ifaddr);

  return exch;
}