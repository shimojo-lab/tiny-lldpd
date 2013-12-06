#ifndef __LLDP_H__
#define __LLDP_H__

enum TLVType
{
    TLV_END = 0,
    TLV_CHASSIS_ID,
    TLV_PORT_ID,
    TLV_TTL,
    TLV_PORT_DESCRIPTION,
    TLV_SYSTEM_NAME,
    TLV_SYSTEM_DESCRIPTION,
    TLV_SYSTEM_CAPABILITIES,
    TLV_MANAGEMENT_ADDRESS,
    TLV_ORGANIZATION_SPECIFIC = 127
};

enum ChassisIDSubType
{
    CHASSIS_ID_CHASSIS_COMPONENT = 1,
    CHASSIS_ID_INTERFACE_ALIAS,
    CHASSIS_ID_PORT_COMPONENT,
    CHASSIS_ID_MAC_ADDRESS,
    CHASSIS_ID_NETWORK_ADDRESS,
    CHASSIS_ID_INTERFACE_NAME,
    CHASSIS_ID_LOCALLY_ASSIGNED
};

enum PortIDSubType
{
    PORT_ID_INTERFACE_ALIAS = 1,
    PORT_ID_PORT_COMPONENT,
    PORT_ID_MAC_ADDRESS,
    PORT_ID_NETWORK_ADDRESS,
    PORT_ID_INTERFACE_NAME,
    PORT_ID_AGENT_CIRCUIT_ID,
    PORT_ID_LOCALLY_ASSIGNED
};

int write_lldp_tlv_header(void *buffer, int type, int length);
int write_lldp_tlv(void *buffer, int type, int length, const void *value);
int write_lldp_chassis_id_tlv(void *buffer, int chassis_id_subtype, int length, const void *chassis_id);
int write_lldp_port_id_tlv(void *buffer, int port_id_subtype, int length, const void *port_id);
int write_lldp_ttl_tlv(void *buffer, int ttl);
int write_lldp_end_tlv(void *buffer);

#endif
