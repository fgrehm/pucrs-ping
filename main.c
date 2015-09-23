#include <stdio.h>
#include <stdlib.h>
#include <net/ethernet.h>
#include <sys/socket.h>

int main() {
  // Creates the raw socket to send packets
  int sock_fd = 0;
  if((sock_fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) < 0) {
    printf("Erro na criacao do socket.\n");
    exit(1);
  }

  /* Set up mac / IPv4 addresses for the machines that will receive the packets */
  // TODO: This should be passed in as arguments to the CLI
  char *local_mac = "E8:B1:FC:00:5D:F2";
  char *local_ip  = "10.2.3.4";
  char *dest_mac  = "28:32:C5:D4:47:8A";
  char *dest_ip   = "11.2.3.4";

  /* The buffer where the message gets built */
  int send_result = send_echo_request_packet(sock_fd, local_ip, local_mac, dest_ip, dest_mac);
  if (send_result < 0) {
      printf("ERROR sending packet!\n");
      exit(1);
  }
  printf("Send success (%d).\n", send_result);
}
