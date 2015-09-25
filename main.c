#include <errno.h>
#include <time.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include "constants.h"
#include "echo_request.h"
#include "echo_reply.h"

int create_socket();
unsigned char *parse_mac_addr(char *mac_str);
unsigned char *parse_ip_addr(char *ip_str);

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

  int i;
  for (i = 0; i < 6; i++) {
    int send_result = send_echo_request_packet(sock_fd, local_ip, local_mac, dest_ip, dest_mac);
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
  strcpy(ifr.ifr_name, "wlan0");
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
