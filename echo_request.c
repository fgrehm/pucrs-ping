#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/if_ether.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>
#include <netinet/ether.h>
#include "echo_request.h"

#define ETHERTYPE_LEN 2
#define MAC_ADDR_LEN 6
#define IP_ADDR_LEN 4
#define BUFFER_LEN 1518

char *write_byte(char *bufferptr, unsigned char byte);
char *write_ip_bytes(char *bufferptr, char *ip_str);
char *write_ethernet(char *bufferptr, char *dest_mac, char *local_mac);
char *write_ipv4(char *bufferptr, char *local_ip, char *dest_ip);
char *write_icmp(char *bufferptr);

int send_packet(int sock_fd, char *dest_mac, char *buffer, int packet_size);

char *parse_mac_addr(char *mac_str);

int send_echo_request_packet(int sock_fd, char *local_ip, char *local_mac_str, char *dest_ip, char *dest_mac_str) {
  char buffer[BUFFER_LEN];
  char* bufferptr = buffer;

  unsigned char *local_mac = parse_mac_addr(local_mac_str);
  unsigned char *dest_mac  = parse_mac_addr(dest_mac_str);

  // Prepare packet data as needed
  bufferptr = write_ethernet(bufferptr, dest_mac, local_mac);
  bufferptr = write_ipv4(bufferptr, local_ip, dest_ip);
  bufferptr = write_icmp(bufferptr);

  // Zero out...
  memset(bufferptr, 0, 64);

  return send_packet(sock_fd, dest_mac, buffer, 42);
}

// Based on http://stackoverflow.com/a/3409211
char *parse_mac_addr(char *mac_str) {
  unsigned char *result = calloc(MAC_ADDR_LEN, sizeof(unsigned char));
  sscanf(mac_str, "%2hhx:%2hhx:%2hhx:%2hhx:%2hhx:%2hhx", result, result + 1, result + 2, result + 3, result + 4, result + 5);
  return result;
}

int send_packet(int sock_fd, char *dest_mac, char *buffer, int packet_size) {
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
char *write_ethernet(char *bufferptr, char *dest_mac, char *local_mac) {
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
  // Grab a reference to the beggining of the header so we can come back and
  // calculate the checksum
  char *start = bufferptr;

  /* IP version 4 and 20 bytes header */
  bufferptr = write_byte(bufferptr, 0x45);

  /* Set services field to zero */
  bufferptr = write_byte(bufferptr, 0x00);

  /* Length of the packet (36 bytes) */
  bufferptr = write_byte(bufferptr, 0x00);
  bufferptr = write_byte(bufferptr, 0x1c);

  /* ID */
  bufferptr = write_byte(bufferptr, 0xFF);
  bufferptr = write_byte(bufferptr, 0xEE);

  /* Flags (don't fragment) */
  bufferptr = write_byte(bufferptr, 0x40);

  /* Offset (zero) */
  bufferptr = write_byte(bufferptr, 0x00);

  /* TTL (64) */
  bufferptr = write_byte(bufferptr, 0x40);

  /* ICMP */
  bufferptr = write_byte(bufferptr, 0x01);

  /* Zeroed checksum */
  char *checksumstart = bufferptr;
  bufferptr = write_byte(bufferptr, 0x00);
  bufferptr = write_byte(bufferptr, 0x00);

  /* Source IP */
  bufferptr = write_ip_bytes(bufferptr, local_ip);

  /* Destination IP */
  bufferptr = write_ip_bytes(bufferptr, dest_ip);

  // Calculate the checksum;
  unsigned short checksum = in_cksum(start, 20);
  memcpy(checksumstart, &checksum, 2);

  return bufferptr;
}

char *write_icmp(char *bufferptr) {
  // Grab a reference to the beggining of the header so we can come back and
  // calculate the checksum
  char *start = bufferptr;

  /* Type (request) */
  bufferptr = write_byte(bufferptr, 0x08);

  /* Code (zero) */
  bufferptr = write_byte(bufferptr, 0x00);

  // Zeroed checksum
  char *checksumstart = bufferptr;
  bufferptr = write_byte(bufferptr, 0x00);
  bufferptr = write_byte(bufferptr, 0x00);

  // Identifier
  // TODO: This should be dynamic
  bufferptr = write_byte(bufferptr, 0x6e);
  bufferptr = write_byte(bufferptr, 0xd2);

  // Sequence number
  // TODO: This should be dynamic
  bufferptr = write_byte(bufferptr, 0x00);
  bufferptr = write_byte(bufferptr, 0x02);

  // Calculate the checksum;
  unsigned short checksum = in_cksum(start, 8);
  memcpy(checksumstart, &checksum, 2);

  return bufferptr;
}
