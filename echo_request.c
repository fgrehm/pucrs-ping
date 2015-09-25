#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/if_ether.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>
#include <netinet/ether.h>
#include "echo_request.h"
#include "constants.h"
#include "checksum.h"

char *write_byte(char *bufferptr, unsigned char byte);
char *write_ethernet(char *bufferptr, unsigned char *dest_mac, unsigned char *local_mac);
char *write_ipv4(char *bufferptr, unsigned char *local_ip, unsigned char *dest_ip);
char *write_icmp(char *bufferptr);

int send_packet(int sock_fd, unsigned char *dest_mac, char *buffer, int packet_size);

int send_echo_request_packet(int sock_fd, unsigned char *local_ip, unsigned char *local_mac, unsigned char *dest_ip, unsigned char *dest_mac) {
  char buffer[BUFFER_LEN];
  char* bufferptr = buffer;

  // Prepare packet data as needed
  bufferptr = write_ethernet(bufferptr, dest_mac, local_mac);
  bufferptr = write_ipv4(bufferptr, local_ip, dest_ip);
  bufferptr = write_icmp(bufferptr);

  return send_packet(sock_fd, dest_mac, buffer, 42);
}

int send_packet(int sock_fd, unsigned char *dest_mac, char *buffer, int packet_size) {
  // Identify the machine (MAC) that is going to receive the message sent.
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

// Write the ethernet headers
char *write_ethernet(char *bufferptr, unsigned char *dest_mac, unsigned char *local_mac) {
  memcpy(bufferptr, dest_mac, MAC_ADDR_LEN);
  bufferptr += MAC_ADDR_LEN;

  memcpy(bufferptr, local_mac, MAC_ADDR_LEN);
  bufferptr += MAC_ADDR_LEN;

  short int etherTypeT = htons(0x0800);
  memcpy(bufferptr, &(etherTypeT), sizeof(etherTypeT));

  return bufferptr + ETHERTYPE_LEN;
}

// Write the IPv4 headers
char *write_ipv4(char *bufferptr, unsigned char *local_ip, unsigned char *dest_ip) {
  // Grab a reference to the beggining of the header so we can come back and
  // calculate the checksum
  char *start = bufferptr;

  // IP version 4 and 20 bytes header
  bufferptr = write_byte(bufferptr, 0x45);

  // Set services field to zero
  bufferptr = write_byte(bufferptr, 0x00);

  // Length of the packet (36 bytes)
  bufferptr = write_byte(bufferptr, 0x00);
  bufferptr = write_byte(bufferptr, 0x1c);

  // ID
  bufferptr = write_byte(bufferptr, 0xFF);
  bufferptr = write_byte(bufferptr, 0xEE);

  // Flags (don't fragment)
  bufferptr = write_byte(bufferptr, 0x40);

  // Offset (zero)
  bufferptr = write_byte(bufferptr, 0x00);

  // TTL (64)
  bufferptr = write_byte(bufferptr, 0x40);

  // ICMP
  bufferptr = write_byte(bufferptr, 0x01);

  // Zeroed checksum
  char *checksumstart = bufferptr;
  bufferptr = write_byte(bufferptr, 0x00);
  bufferptr = write_byte(bufferptr, 0x00);

  // Source IP
  memcpy(bufferptr, local_ip, IP_ADDR_LEN);
  bufferptr = bufferptr + IP_ADDR_LEN;

  // Destination IP
  memcpy(bufferptr, dest_ip, IP_ADDR_LEN);
  bufferptr = bufferptr + IP_ADDR_LEN;

  // Calculate the checksum
  unsigned short checksum = in_cksum((short unsigned int *)start, 20);
  memcpy(checksumstart, &checksum, 2);

  return bufferptr;
}

char *write_icmp(char *bufferptr) {
  // Grab a reference to the beggining of the header so we can come back and
  // calculate the checksum
  char *start = bufferptr;

  // Type (echo request)
  bufferptr = write_byte(bufferptr, 0x08);

  // Code (zero)
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
  unsigned short checksum = in_cksum((short unsigned int *)start, 8);
  memcpy(checksumstart, &checksum, 2);

  return bufferptr;
}
