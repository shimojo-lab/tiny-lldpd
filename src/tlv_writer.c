#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/utsname.h>
#include <net/ethernet.h>
#include <netinet/in.h>
#include <netpacket/packet.h>

#include "lldp.h"
#include "tlv_writer.h"

int write_ethernet_header(void *buffer, const char *src_address, const char *dst_address, int protocol_type)
{
    struct ether_header *eh = (struct ether_header *)buffer;

    memcpy(eh->ether_shost, src_address, ETH_ALEN);
    memcpy(eh->ether_dhost, dst_address, ETH_ALEN);
    eh->ether_type = htons(protocol_type);
    
    return sizeof(struct ether_header);
}

int write_system_name_tlv(void *buffer)
{
    char hostname[HOST_NAME_LENGTH];
    gethostname(hostname, HOST_NAME_LENGTH);

    return write_lldp_tlv(buffer, TLV_SYSTEM_NAME, strlen(hostname), hostname);
}

int write_system_description_tlv(void *buffer)
{
    struct utsname info;
    uname(&info);

    char tmp[255];
    sprintf(tmp, "%s %s %s %s %s", info.sysname, info.nodename, info.release, info.version, info.machine);

    return write_lldp_tlv(buffer, TLV_SYSTEM_DESCRIPTION, strlen(tmp), tmp);
}

int write_management_address_tlv(void *buffer, int if_ip_address, int if_index)
{
    // Awful code. Must be fixed.
    char tmp[255];
    tmp[0] = 5;
    tmp[1] = 1;
    *((int *)(tmp + 2)) = if_ip_address;
    tmp[6] = 2;
    *((int *)(tmp + 7)) = htonl(if_index);
    tmp[11] = 0;

    return write_lldp_tlv(buffer, TLV_MANAGEMENT_ADDRESS, 12, tmp);
}

int write_port_description_tlv(void *buffer, const char *if_name)
{
    return write_lldp_tlv(buffer, TLV_PORT_DESCRIPTION, strlen(if_name), if_name);
}
