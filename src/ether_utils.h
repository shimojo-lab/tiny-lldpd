#ifndef __ETHER_UTILS_H__
#define __ETHER_UTILS_H__

#include <stdint.h>

int get_if_index(const char *if_name, int sock, int *index);
int get_if_mac_addres(const char *if_name, int sock, char *mac_address);
int get_if_ip_addres(const char *if_name, int sock, int *ip_address);
int get_all_ifs(int sock, char (*if_names)[16], int *if_count);

#endif
