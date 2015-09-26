#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "echo_request.h"
#include "echo_reply.h"
#include "constants.h"
#include "io.h"

reply_response_t wait_for_icmp_reply(int sock_fd, echo_request_t req) {
  unsigned char buffer[BUFFER_LEN*2];
  unsigned char ttl;

  for (;;) {
    recv(sock_fd,(char *) &buffer, sizeof(buffer), 0x0);

    // Check if destination MAC address is our interface
    int i, match = 1;
    for (i = 0; i < MAC_ADDR_LEN; i++) {
      if (buffer[i] != req.local_mac[i]) {
        match = 0;
        break;
      }
    }

    printf("match: %d\n", match);

    if (match != 1)
      continue;

    unsigned char *bufferptr = buffer;

    // Move past the 2 MAC addresses on the ethernet header
    bufferptr += 2 * MAC_ADDR_LEN;

    // Check if the packet type is an IPv4
    short ether_type;
    CONSUME_BYTES(bufferptr, &ether_type, sizeof(ether_type));
    printf("ether type: %d\n", ether_type);
    if (ether_type != 8) // IP
      continue;

    // Reference to where the IP section starts
    unsigned char *ip_start_ptr = bufferptr;

    // Check if is V4 and has 20 bytes on the header
    char ip_version_and_header_length;
    CONSUME_BYTES(bufferptr, &ip_version_and_header_length, sizeof(ip_version_and_header_length));
    printf("version and header length: %x\n", ip_version_and_header_length);
    if (ip_version_and_header_length != 0x45)
      continue;

    // Move forward 1 byte that represents some flags we don't care about
    bufferptr += 1;

    // Total length of the packet
    unsigned short total_packet_length;
    CONSUME_BYTES(bufferptr, &total_packet_length, sizeof(total_packet_length));
    total_packet_length = ntohs(total_packet_length);
    printf("  - total packet length %d\n", total_packet_length);

    // Move forward some bytes that represents some stuff we don't care about
    bufferptr += 4; // 2 for packet ID, 2 for flags and offset

    // Extract ttl
    CONSUME_BYTES(bufferptr, &ttl, sizeof(ttl));
    printf("  - ttl %d\n", ttl);

    // Check if the protocol is ICMP
    unsigned char protocol;
    CONSUME_BYTES(bufferptr, &protocol, sizeof(protocol));
    printf("  - protocol %d\n", protocol);
    if (protocol != 1)
      continue;

    // Is the checksum for the IP header correct?
    unsigned short checksum;
    unsigned char *checksum_ptr = bufferptr;
    CONSUME_BYTES(bufferptr, &checksum, sizeof(checksum));
    checksum = ntohs(checksum);
    printf("  - checksum %x\n", checksum);
    // Go back to the checksum start and zero it out in order to validate the message
    memset(checksum_ptr, 0, sizeof(checksum));
    // Calculate the checksum with the info we have
    unsigned short calculated_checksum = in_cksum(ip_start_ptr, IP_HEADER_LEN);
    calculated_checksum = ntohs(calculated_checksum);
    printf("  - calculated checksum %x\n", calculated_checksum);

    if (checksum != calculated_checksum)
      continue;

    // Source IP
    unsigned char source_ip[IP_ADDR_LEN];
    CONSUME_BYTES(bufferptr, source_ip, IP_ADDR_LEN * sizeof(unsigned char));
    printf("  - source ip %d.%d.%d.%d\n", source_ip[0], source_ip[1], source_ip[2], source_ip[3]);

    // Target IP (us)
    unsigned char target_ip[IP_ADDR_LEN];
    CONSUME_BYTES(bufferptr, target_ip, IP_ADDR_LEN * sizeof(unsigned char));
    printf("  - target ip %d.%d.%d.%d\n", target_ip[0], target_ip[1], target_ip[2], target_ip[3]);
    // Does the IP match?
    for (i = 0; i < IP_ADDR_LEN; i++) {
      if (target_ip[i] != req.local_ip[i]) {
        match = 0;
        break;
      }
    }
    if (match != 1)
      continue;

    // Check if the reply is for us

    // TODO: Calculate time elapsed

    if (match) {
      printf("  - TODO: Finish parsing echo reply\n");
      // Verifica se protocolo eh ICMP
      // Verifica se identificador eh igual ao enviado
      // Calcula o q tem q calcular
      break;
    }
  }

  reply_response_t res = {
    .success = 1
  };

  return res;
}
