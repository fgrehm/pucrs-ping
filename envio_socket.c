#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/if_ether.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>
#include <netinet/ether.h>

#define ETHERTYPE_LEN 2
#define MAC_ADDR_LEN 6
#define IP_ADDR_LEN 4
#define BUFFER_LEN 1518

// Atencao!! Confira no /usr/include do seu sisop o nome correto
// das estruturas de dados dos protocolos.

typedef unsigned char MacAddress[MAC_ADDR_LEN];
extern int errno;

void write_ip_bytes(char *ip, unsigned char *buffer);
int send_packet(int sock_fd, MacAddress dest_mac, char *buffer, int packet_size);

int main()
{
  // Creates the raw socket to send packets
  int sock_fd = 0;
  if((sock_fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) < 0) {
    printf("Erro na criacao do socket.\n");
    exit(1);
  }

  /* Set up mac / IPv4 addresses for the machines that will receive the packets */
  // TODO: This should be passed in as arguments to the CLI
  MacAddress local_mac = {0xE8, 0xB1, 0xFC, 0x00, 0x5D, 0xF2};
  char *local_ip       = "10.2.3.4";
  MacAddress dest_mac  = {0x28, 0x32, 0xC5, 0xD4, 0x47, 0x8A};
  char *dest_ip        = "11.2.3.4";

  /* The buffer where the message gets built */
  char buffer[BUFFER_LEN];
  char* bufferptr = buffer;

  /* Ethernet header */
  short int etherTypeT = htons(0x0800);
  memcpy(bufferptr, dest_mac, MAC_ADDR_LEN);
  bufferptr += MAC_ADDR_LEN;
  memcpy(bufferptr, local_mac, MAC_ADDR_LEN);
  bufferptr += MAC_ADDR_LEN;
  memcpy(bufferptr, &(etherTypeT), sizeof(etherTypeT));
  bufferptr += ETHERTYPE_LEN;

  /* IPv4 */
  /* IP version 4 and 20 bytes header */
  memset(bufferptr, 0x45, 1);
  bufferptr += 1;
  /* Set services field to zero */
  memset(bufferptr, 0x00, 1);
  bufferptr += 1;
  /* Length of the packet (84 bytes) */
  memset(bufferptr, 0x00, 1);
  bufferptr += 1;
  memset(bufferptr, 0x54, 1);
  bufferptr += 1;
  /* ID */
  memset(bufferptr, 0x00, 1);
  bufferptr += 1;
  memset(bufferptr, 0x10, 1);
  bufferptr += 1;
  /* Flags (don't fragment) */
  memset(bufferptr, 0x40, 1);
  bufferptr += 1;
  /* Offset (zero) */
  memset(bufferptr, 0x00, 1);
  bufferptr += 1;
  /* TTL (64) */
  memset(bufferptr, 0x40, 1);
  bufferptr += 1;
  /* ICMP */
  memset(bufferptr, 0x01, 1);
  bufferptr += 1;
  /* Checksum (enable checksum) */
  memset(bufferptr, 0x00, 1);
  bufferptr += 1;
  memset(bufferptr, 0x00, 1);
  bufferptr += 1;
  /* Source IP */
  write_ip_bytes(local_ip, bufferptr);
  bufferptr += IP_ADDR_LEN;
  /* Destination IP */
  write_ip_bytes(dest_ip, bufferptr);
  bufferptr += IP_ADDR_LEN;

  /* ICMP */
  /* Type (request) */
  memset(bufferptr, 0x08, 1);
  bufferptr += 1;
  /* Code (zero) */
  memset(bufferptr, 0x00, 1);
  bufferptr += 1;
  /* TODO: Proper checksum */
  memset(bufferptr, 0xf7, 1);
  bufferptr += 1;
  memset(bufferptr, 0xff, 1);
  bufferptr += 1;

  // Zero out...
  memset(bufferptr, 0, 64);

  int send_result = send_packet(sock_fd, dest_mac, buffer, 98);
  if (send_result < 0) {
      printf("ERROR sending packet!\n");
      exit(1);
  }

  printf("Send success (%d).\n", send_result);
}

// Based on http://stackoverflow.com/a/9211667
void write_ip_bytes(char *ip_str, unsigned char *buffer) {
  sscanf(ip_str, "%hhd.%hhd.%hhd.%hhd", buffer, buffer + 1, buffer + 2, buffer + 3);
}

int send_packet(int sock_fd, MacAddress dest_mac, char *buffer, int packet_size) {
  /* Identify the machine (MAC) that is going to receive the message sent. */
  struct sockaddr_ll dest_addr;
  dest_addr.sll_family = htons(PF_PACKET);
  dest_addr.sll_protocol = htons(ETH_P_ALL);
  dest_addr.sll_halen = 6;
  dest_addr.sll_ifindex = 2; // TODO: Parameterize this
  memcpy(&(dest_addr.sll_addr), dest_mac, MAC_ADDR_LEN);

  // Send the actual packet
  return sendto(sock_fd, buffer, packet_size, 0, (struct sockaddr *)&(dest_addr), sizeof(struct sockaddr_ll));
}
