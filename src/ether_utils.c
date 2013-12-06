#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <net/ethernet.h>
#include <netinet/in.h>

#include "ether_utils.h"

int get_if_index(const char *if_name, int sock, int *index)
{
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(struct ifreq));
    strcpy(ifr.ifr_name, if_name);

    if (ioctl(sock, SIOCGIFINDEX, &ifr) < 0) {
        return -1;
    }

    *index = ifr.ifr_ifindex;

    return 0;
}

int get_if_mac_addres(const char *if_name, int sock, char *mac_address)
{
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(struct ifreq));
    strcpy(ifr.ifr_name, if_name);

    if (ioctl(sock, SIOCGIFHWADDR, &ifr) < 0) {
        return -1;
    }

    memcpy(mac_address, ifr.ifr_hwaddr.sa_data, ETH_ALEN);

    return 0;
}

int get_if_ip_addres(const char *if_name, int sock, int *ip_address)
{
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(struct ifreq));
    strcpy(ifr.ifr_name, if_name);

    if (ioctl(sock, SIOCGIFADDR, &ifr) < 0) {
        return -1;
    }

    struct sockaddr_in *addr_in = (struct sockaddr_in *)&ifr.ifr_addr;
    *ip_address = addr_in->sin_addr.s_addr;

    return 0;
}

int get_all_ifs(int sock, char (*if_names)[16], int *if_count)
{
    struct ifconf conf;
    struct ifreq req[16];

    conf.ifc_len = sizeof(req);
    conf.ifc_buf = (__caddr_t)req;
    if (ioctl(sock, SIOCGIFCONF, &conf)) {
        return -1;
    }

    int i;
    int count = 0;
    for (i = 0; i < conf.ifc_len / sizeof(struct ifreq); i++) {
        if (ioctl(sock, SIOCGIFFLAGS, &req[i]) < 0) {
            continue;
        }

        uint16_t flags = req[i].ifr_flags;
        // If interface is up and not a loopback device
        if (!(flags & IFF_LOOPBACK) && (flags & IFF_UP)) {
            strcpy(if_names[count++], req[i].ifr_name);
        }
    }

    *if_count = count;

    return 0;
}
