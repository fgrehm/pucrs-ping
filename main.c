#include <errno.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include "constants.h"
#include "echo_request.h"
#include "echo_reply.h"

void create_socket();
int send_packet(unsigned char *dest_mac, char *buffer, int packet_size);
void *wait_for_icmp_reply_and_mark_as_done(void *args);
reply_response_t wait_for_icmp_reply_or_timeout(echo_request_t req);

unsigned char *parse_mac_addr(char *mac_str);
unsigned char *parse_ip_addr(char *ip_str);

int sock_fd = 0;
struct timespec max_wait;

int main(int argc, char *argv[]) {
  if (argc != 5) {
    printf("Usage: pucrs-ping LOCAL_MAC_ADDR LOCAL_IP_ADDR DESTINATION_MAC_ADDR DESTINATION_IP_ADDR\n\n");
    exit(1);
  }

  create_socket();

  // Set up mac / IPv4 addresses for the machines that will receive the packets
  char *local_mac_str = argv[1];
  char *local_ip_str  = argv[2];
  char *dest_mac_str  = argv[3];
  char *dest_ip_str   = argv[4];

  // Set up timeout stuff
  memset(&max_wait, 0, sizeof(max_wait));
  max_wait.tv_sec = MAX_WAIT_SEC;

  // Convert input to bytes
  unsigned char *local_mac = parse_mac_addr(local_mac_str);
  unsigned char *local_ip  = parse_ip_addr(local_ip_str);
  unsigned char *dest_mac  = parse_mac_addr(dest_mac_str);
  unsigned char *dest_ip   = parse_ip_addr(dest_ip_str);

  // This helps us identify our requests
  unsigned short identifier = getpid();

  int i;
  for (i = 0; i < TOTAL_PACKETS; i++) {
    echo_request_t req = prepare_echo_request(identifier, local_ip, local_mac, dest_ip, dest_mac);
    int send_result = send_packet(dest_mac, req.raw_packet, BUFFER_LEN);
    if (send_result < 0) {
      printf("ERROR sending packet: %d\n", send_result);
      exit(1);
    }
    printf("\n[%d] Send success (%d).\n", i+1, send_result);

    printf("  - TODO: Implement timeout.\n");
    reply_response_t res = wait_for_icmp_reply(sock_fd, req);
    if (res.success) {
      printf("  - TODO: Print statistics.\n");
      continue;
    }

    if (res.timed_out) {
      printf("  - Timeout.\n");
    } else if (res.ttl_exceeded) {
      printf("  - TTL exceeded (%s).\n", res.source_ip);
    } else {
      printf("ERROR!.\n");
      exit(1);
    }
  }

  return 0;
}

void create_socket() {
  // Creates the raw socket to send packets
  if((sock_fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) < 0) {
    printf("Erro na criacao do socket.\n");
    exit(1);
  }

  // Set interface to promiscuous mode
  struct ifreq ifr;
  strcpy(ifr.ifr_name, INTERFACE_NAME);
  if(ioctl(sock_fd, SIOCGIFINDEX, &ifr) < 0) {
    printf("ioctl error!");
    exit(1);
  }
  ioctl(sock_fd, SIOCGIFFLAGS, &ifr);
  ifr.ifr_flags |= IFF_PROMISC;
  ioctl(sock_fd, SIOCSIFFLAGS, &ifr);
}

// Based on http://stackoverflow.com/a/3409211
unsigned char *parse_mac_addr(char *mac_str) {
  unsigned char *result = calloc(MAC_ADDR_LEN, sizeof(unsigned char));
  sscanf(mac_str, "%2hhx:%2hhx:%2hhx:%2hhx:%2hhx:%2hhx", result, result + 1, result + 2, result + 3, result + 4, result + 5);
  return result;
}

// Based on http://stackoverflow.com/a/9211667
unsigned char *parse_ip_addr(char *ip_str) {
  unsigned char *bytes = calloc(IP_ADDR_LEN, sizeof(unsigned char));
  sscanf(ip_str, "%hhd.%hhd.%hhd.%hhd", bytes, bytes + 1, bytes + 2, bytes + 3);
  return bytes;
}

int send_packet(unsigned char *dest_mac, char *buffer, int packet_size) {
  // Identify the machine (MAC) that is going to receive the message sent.
  struct sockaddr_ll dest_addr;
  dest_addr.sll_family = htons(PF_PACKET);
  dest_addr.sll_protocol = htons(ETH_P_ALL);
  dest_addr.sll_halen = 6;
  dest_addr.sll_ifindex = INTERFACE_INDEX;
  memcpy(&(dest_addr.sll_addr), dest_mac, MAC_ADDR_LEN);

  // Send the actual packet
  return sendto(sock_fd, buffer, packet_size, 0, (struct sockaddr *)&(dest_addr), sizeof(struct sockaddr_ll));
}
