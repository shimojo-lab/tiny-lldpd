#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <syslog.h>
#include <unistd.h>
#include <signal.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>

#include "lldp.h"
#include "ether_utils.h"
#include "tlv_writer.h"

#define PID_FILE_PATH       "/var/run/tlldpd.pid"
#define SENDBUF_SIZE        (1024)
#define ETH_P_LLDP          (0x88cc)

int check_duplicate_run()
{
    FILE *pid_file;
    if ((pid_file = fopen(PID_FILE_PATH, "r"))) {
        int pid;
        if (fscanf(pid_file, "%d", &pid) == 1) {
            if (kill(pid, 0) >= 0) {
                return 1;
            }
        }
        fclose(pid_file);
    }

    return 0;
}

int daemonize()
{
    // If already a daemon
    if (getppid() == 1) {
        return 0;
    }

    if (daemon(0, 0) < 0) {
        return -1;
    }

    openlog("tlldpd", LOG_PID, LOG_DAEMON);
    syslog(LOG_INFO, "Tiny LLDP daemon started.\n");

    pid_t pid = getpid();
    FILE *pid_file;
    if ((pid_file = fopen(PID_FILE_PATH, "w"))) {
        fprintf(pid_file, "%d\n", pid);
        fclose(pid_file);
    } else {
        syslog(LOG_ERR, "Failed to write pid file.\n");
    }

    return 0;
}

void send_lldp_packet(const char *if_name, int sock)
{
    // Group MAC address for neareset bridge
    const char dst_address[ETH_ALEN] = {0x01, 0x80, 0xc2, 0x00, 0x00, 0x0e};
    char src_address[ETH_ALEN];
    int if_ip_address;
    int if_index;

    if (get_if_mac_addres(if_name, sock, src_address) < 0) {
        syslog(LOG_ERR, "Failed to get interface MAC address\n");
    }
    if (get_if_ip_addres(if_name, sock, &if_ip_address) < 0) {
        syslog(LOG_ERR, "Failed to get interface IP address\n");
    }
    if (get_if_index(if_name, sock, &if_index) < 0) {
        syslog(LOG_ERR, "Failed to get interface index\n");
    }

    // Construct Ethernet frame
    int frame_size = 0;
    char sendbuf[SENDBUF_SIZE];
    memset(sendbuf, 0, SENDBUF_SIZE);

    // Construct Ethernet header
    frame_size += write_ethernet_header(sendbuf, src_address, dst_address, ETH_P_LLDP);

    // Construct Ethernet body

    // Required LLDP TLVs
    frame_size += write_lldp_chassis_id_tlv(sendbuf + frame_size, CHASSIS_ID_MAC_ADDRESS, ETH_ALEN, src_address);
    frame_size += write_lldp_port_id_tlv(sendbuf + frame_size, PORT_ID_MAC_ADDRESS, ETH_ALEN, src_address);
    frame_size += write_lldp_ttl_tlv(sendbuf + frame_size, LLDP_DEFAULT_TTL);

    // Optionsl LLDP TLVs
    frame_size += write_port_description_tlv(sendbuf + frame_size, if_name);
    frame_size += write_system_name_tlv(sendbuf + frame_size);
    frame_size += write_system_description_tlv(sendbuf + frame_size);
    frame_size += write_management_address_tlv(sendbuf + frame_size, if_ip_address, if_index);
    
    frame_size += write_lldp_end_tlv(sendbuf + frame_size);

    // Send out Ethernet frame
    struct sockaddr_ll socket_address;
    socket_address.sll_ifindex = if_index;
    socket_address.sll_halen = ETH_ALEN;
    memcpy(socket_address.sll_addr, dst_address, ETH_ALEN);
    if (sendto(sock, sendbuf, frame_size, 0, (struct sockaddr*)&socket_address, sizeof(struct sockaddr_ll)) < 0) {
        syslog(LOG_ERR, "LLDP packet send failed\n");
    }
}

int main(int argc, char *argv[])
{
    // flags
    int duplicate_run_allowed = 0;
    int lldp_interval = 1;
    int do_daemonize = 1;

    // Comannd line options parsing
    int result;
    while((result = getopt(argc, argv, "Ddi:")) != -1) {
        switch(result) {
            case 'd':
                duplicate_run_allowed = 1;
            break;
            case 'i':
                lldp_interval = atoi(optarg);
            break;
            case 'D':
                do_daemonize = 0;
            break;
        }
    }

    // Check if root
    if (getuid() != 0) {
        printf("You must be root to run this daemon\n");
        exit(EXIT_FAILURE);
    }

    // Check if a tiny-lldpd is running
    if (!duplicate_run_allowed && check_duplicate_run()) {
        printf("tiny-lldpd daemon is already running\n");
        exit(EXIT_FAILURE);
    }

    // Daemoniza myself
    printf("Starting tiny-lldpd...\n");
    if (do_daemonize && daemonize() < 0) {
        printf("Failed to daemonize\n");
        exit(EXIT_FAILURE);
    }

    // Get socket
    int sock = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (sock < 0) {
        syslog(LOG_ERR, "Socket initialization error\n");
        exit(EXIT_FAILURE);
    }

    char if_names[16][16];
    int if_count;
    // Get all interface names
    get_all_ifs(sock, if_names, &if_count);

    while (1) {
        int i;
        // For each interfaces
        for (i = 0; i < if_count; i ++) {
            // Send out LLDP packet
            send_lldp_packet(if_names[i], sock);
        }

        sleep(lldp_interval);
    }

    return 0;
}
