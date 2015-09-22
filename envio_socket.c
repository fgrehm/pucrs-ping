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

char *write_byte(char *bufferptr, unsigned char byte);
char *write_ip_bytes(char *bufferptr, char *ip_str);
char *write_ethernet(char *bufferptr, MacAddress dest_mac, MacAddress local_mac);
char *write_ipv4(char *bufferptr, char *local_ip, char *dest_ip);
char *write_icmp(char *bufferptr);
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
  int increment = 0;

  // Prepare packet data as needed
  bufferptr = write_ethernet(bufferptr, dest_mac, local_mac);
  bufferptr = write_ipv4(bufferptr, local_ip, dest_ip);
  bufferptr = write_icmp(bufferptr);

  // Zero out...
  memset(bufferptr, 0, 64);

  int send_result = send_packet(sock_fd, dest_mac, buffer, 98);
  if (send_result < 0) {
      printf("ERROR sending packet!\n");
      exit(1);
  }
  printf("Send success (%d).\n", send_result);
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

char *write_byte(char *bufferptr, unsigned char byte) {
  memset(bufferptr, byte, 1);
  return bufferptr + 1;
}

// Based on http://stackoverflow.com/a/9211667
char *write_ip_bytes(char *bufferptr, char *ip_str) {
  sscanf(ip_str, "%hhd.%hhd.%hhd.%hhd", bufferptr, bufferptr + 1, bufferptr + 2, bufferptr + 3);
  return bufferptr + IP_ADDR_LEN;
}

// Write the ethernet headers
char *write_ethernet(char *bufferptr, MacAddress dest_mac, MacAddress local_mac) {
  memcpy(bufferptr, dest_mac, MAC_ADDR_LEN);
  bufferptr += MAC_ADDR_LEN;

  memcpy(bufferptr, local_mac, MAC_ADDR_LEN);
  bufferptr += MAC_ADDR_LEN;

  short int etherTypeT = htons(0x0800);
  memcpy(bufferptr, &(etherTypeT), sizeof(etherTypeT));

  return bufferptr + ETHERTYPE_LEN;
}

// Write the IPv4 headers
char *write_ipv4(char *bufferptr, char *local_ip, char *dest_ip) {
  /* IP version 4 and 20 bytes header */
  bufferptr = write_byte(bufferptr, 0x45);

  /* Set services field to zero */
  bufferptr = write_byte(bufferptr, 0x00);

  /* Length of the packet (84 bytes) */
  bufferptr = write_byte(bufferptr, 0x00);
  bufferptr = write_byte(bufferptr, 0x54);

  /* ID */
  bufferptr = write_byte(bufferptr, 0x00);
  bufferptr = write_byte(bufferptr, 0x10);

  /* Flags (don't fragment) */
  bufferptr = write_byte(bufferptr, 0x40);

  /* Offset (zero) */
  bufferptr = write_byte(bufferptr, 0x00);

  /* TTL (64) */
  bufferptr = write_byte(bufferptr, 0x40);

  /* ICMP */
  bufferptr = write_byte(bufferptr, 0x01);

  /* Checksum (TODO: enable checksum) */
  bufferptr = write_byte(bufferptr, 0x00);
  bufferptr = write_byte(bufferptr, 0x00);

  /* Source IP */
  bufferptr = write_ip_bytes(bufferptr, local_ip);

  /* Destination IP */
  return write_ip_bytes(bufferptr, dest_ip);
}

char *write_icmp(char *bufferptr) {
  /* Type (request) */
  bufferptr = write_byte(bufferptr, 0x08);

  /* Code (zero) */
  bufferptr = write_byte(bufferptr, 0x00);

  /* TODO: Proper checksum */
  bufferptr = write_byte(bufferptr, 0xf7);
  return write_byte(bufferptr, 0xff);
}
