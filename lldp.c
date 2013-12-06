#include <stdint.h>
#include <string.h>
#include <netinet/in.h>

#include "lldp.h"

int write_lldp_tlv_header(void *buffer, int type, int length)
{
    *((uint16_t *)buffer) = htons((type & 0x7f) << 9 | (length & 0x1ff));

    return 2;
}

int write_lldp_tlv(void *buffer, int type, int length, const void *value)
{
    int size = 0;

    size += write_lldp_tlv_header(buffer, type, length);
    memcpy(buffer + size, value, length);
    size += length;

    return size;
}

int write_lldp_chassis_id_tlv(void *buffer, int chassis_id_subtype, int length, const void *chassis_id)
{
    int size = 0;

    size += write_lldp_tlv_header(buffer, TLV_CHASSIS_ID, length + 1);
    *((uint8_t *)buffer + size) = (uint8_t)chassis_id_subtype;
    size++;
    memcpy(buffer + size, chassis_id, length);
    size += length;

    return size;
}

int write_lldp_port_id_tlv(void *buffer, int port_id_subtype, int length, const void *port_id)
{
    int size = 0;

    size += write_lldp_tlv_header(buffer, TLV_PORT_ID, length + 1);
    *((uint8_t *)buffer + size) = (uint8_t)port_id_subtype;
    size++;
    memcpy(buffer + size, port_id, length);
    size += length;

    return size;
}

int write_lldp_ttl_tlv(void *buffer, int ttl)
{
    int size = 0;

    size += write_lldp_tlv_header(buffer, TLV_TTL, 2);
    *((uint16_t *)(buffer + size)) = htons((uint16_t)120);
    size += 2;

    return size;
}

int write_lldp_end_tlv(void *buffer)
{
    int size = 0;

    size += write_lldp_tlv_header(buffer, TLV_END, 0);

    return size;
}
