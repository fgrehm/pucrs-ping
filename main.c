#include <errno.h>
#include <time.h>
#include <pthread.h>
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

int create_socket();
int send_packet(int sock_fd, unsigned char *dest_mac, char *buffer, int packet_size);

unsigned char *parse_mac_addr(char *mac_str);
unsigned char *parse_ip_addr(char *ip_str);

// void wait_for_icmp_reply_or_timeout(struct timespec *max_wait, int sock_fd, char *local_mac, char *local_ip, char *dest_mac, char *dest_ip);

// pthread_mutex_t waiting = PTHREAD_MUTEX_INITIALIZER;
// pthread_cond_t done = PTHREAD_COND_INITIALIZER;

int main() {
  int sock_fd = create_socket();

  /* Set up mac / IPv4 addresses for the machines that will receive the packets */
  // TODO: This should be passed in as arguments to the CLI
  char *local_mac_str = "E8:B1:FC:00:5D:F2";
  char *local_ip_str  = "192.168.0.12";
  char *dest_mac_str  = "28:32:C5:D4:47:8A";
  char *dest_ip_str   = "192.168.0.1";

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
    int send_result = send_packet(sock_fd, dest_mac, req.raw_packet, BUFFER_LEN);
    if (send_result < 0) {
      printf("ERROR sending packet!\n");
      exit(1);
    }
    printf("Send success (%d).\n", send_result);
    wait_for_icmp_reply(sock_fd, local_ip, local_mac, dest_ip, dest_mac);
  }

  return 0;
}

int create_socket() {
  int sock_fd = 0;

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

  return sock_fd;
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

int send_packet(int sock_fd, unsigned char *dest_mac, char *buffer, int packet_size) {
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
