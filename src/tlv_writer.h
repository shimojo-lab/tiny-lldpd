#ifndef __TLV_WRITER_H__
#define __TLV_WRITER_H__

#define HOST_NAME_LENGTH    (255)
#define LLDP_DEFAULT_TTL    (120)

int write_ethernet_header(void *buffer, const char *src_address, const char *dst_address, int protocol_type);
int write_system_name_tlv(void *buffer);
int write_system_description_tlv(void *buffer);
int write_management_address_tlv(void *buffer, int if_ip_address, int if_index);
int write_port_description_tlv(void *buffer, const char *if_name);

#endif
