#ifndef CONSTANTS_H
#define CONSTANTS_H

#define ETHERTYPE_LEN 2
#define ETHERNET_LEN (2 * MAC_ADDR_LEN + ETHERTYPE_LEN)
#define MAC_ADDR_LEN 6
#define IP_ADDR_LEN 4
#define IP_HEADER_LEN 20
#define IP_LEN 28
#define BUFFER_LEN 42
#define ICMP_LEN 8
#define INTERFACE_INDEX 2
#define INTERFACE_NAME "wlan0"
#define TTL 64
#define TOTAL_PACKETS 6
#define MAX_WAIT_SEC 2

#endif
