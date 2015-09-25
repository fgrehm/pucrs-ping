#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include "echo_request.h"
#include "echo_reply.h"

int main() {
  // Creates the raw socket to send packets
  int sock_fd = 0;
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

  /* Set up mac / IPv4 addresses for the machines that will receive the packets */
  // TODO: This should be passed in as arguments to the CLI
  char *local_mac = "E8:B1:FC:00:5D:F2";
  char *local_ip  = "192.168.0.12";
  char *dest_mac  = "28:32:C5:D4:47:8A";
  char *dest_ip   = "192.168.0.1";

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
