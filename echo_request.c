#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include "echo_request.h"
#include "constants.h"
#include "checksum.h"
#include "io.h"

char *write_ethernet(char *bufferptr, unsigned char *dest_mac, unsigned char *local_mac);
char *write_ipv4(char *bufferptr, unsigned char *local_ip, unsigned char *dest_ip);
char *write_icmp(char *bufferptr, unsigned short identifier);

unsigned short sequence_number = 1;

echo_request_t prepare_echo_request(unsigned short identifier, unsigned char *local_ip, unsigned char *local_mac, unsigned char *dest_ip, unsigned char *dest_mac) {
  char *buffer = calloc(sizeof(char), BUFFER_LEN);
  char *bufferptr = buffer;

  // Prepare packet data as needed
  bufferptr = write_ethernet(bufferptr, dest_mac, local_mac);
  bufferptr = write_ipv4(bufferptr, local_ip, dest_ip);
  bufferptr = write_icmp(bufferptr, identifier);

  echo_request_t req = {
    .identifier      = identifier,
    .sequence_number = sequence_number++,
    .timestamp       = 0,
    .local_mac       = local_mac,
    .local_ip        = local_ip,
    .raw_packet      = buffer
  };

  return req;
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
  APPEND_BYTE(bufferptr, 0x45);

  // Set services field to zero
  APPEND_BYTE(bufferptr, 0);

  // Length of the packet (28 bytes)
  APPEND_SHORT(bufferptr, IP_LEN);

  // ID
  APPEND_SHORT(bufferptr, 1000);

  // Flags (don't fragment)
  APPEND_BYTE(bufferptr, 0x40);

  // Offset (zero)
  APPEND_BYTE(bufferptr, 0);

  // TTL
  APPEND_BYTE(bufferptr, TTL);

  // ICMP
  APPEND_BYTE(bufferptr, 1);

  // Zeroed checksum
  char *checksumstart = bufferptr;
  APPEND_SHORT(bufferptr, 0);

  // Source IP
  APPEND_BYTES(bufferptr, local_ip, IP_ADDR_LEN);

  // Destination IP
  APPEND_BYTES(bufferptr, dest_ip, IP_ADDR_LEN);

  // Calculate the checksum
  unsigned short checksum = in_cksum((short unsigned int *)start, IP_HEADER_LEN);
  memcpy(checksumstart, &checksum, 2);

  return bufferptr;
}

char *write_icmp(char *bufferptr, unsigned short identifier) {
  // Grab a reference to the beggining of the header so we can come back and
  // calculate the checksum
  char *start = bufferptr;

  // Type (echo request)
  APPEND_BYTE(bufferptr, 8);

  // Code (zero)
  APPEND_BYTE(bufferptr, 0);

  // Zeroed checksum
  char *checksumstart = bufferptr;
  APPEND_SHORT(bufferptr, 0);

  // Identifier
  APPEND_SHORT(bufferptr, identifier);

  // Sequence number
  APPEND_SHORT(bufferptr, sequence_number);

  // Calculate the checksum;
  unsigned short checksum = in_cksum((short unsigned int *)start, ICMP_LEN);
  memcpy(checksumstart, &checksum, 2);

  return bufferptr;
}
