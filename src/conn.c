#include "../include/conn.h"

int host_interfaces(void)
{
  struct ifaddrs *ifaddr, *ifa;
  char addr[INET_ADDRSTRLEN];

  if (getifaddrs(&ifaddr) == -1)
  {
    return -1;
  }

  for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
  {
    if (ifa->ifa_addr == NULL)
    {
      continue;
    }

    int family = ifa->ifa_addr->sa_family;
    if (family != AF_INET)
    {
      continue;
    }

    struct sockaddr_in *ipv4 = (struct sockaddr_in *)ifa->ifa_addr;

    inet_ntop(AF_INET, (void *)&ipv4->sin_addr, addr, sizeof(addr));
    printf("%s: %s\n", ifa->ifa_name, addr);
  }

  freeifaddrs(ifaddr);
  return 0;
}