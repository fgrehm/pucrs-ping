#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include "checksum.h"
#include "echo_request.h"
#include "echo_reply.h"
#include "constants.h"
#include "io.h"

unsigned char ethernet_valid(unsigned char *packet, echo_request_t req);
unsigned char ipv4_valid(unsigned char *packet, echo_request_t req, reply_response_t *res);
unsigned char icmp_valid(unsigned char *packet, echo_request_t req, reply_response_t *res);

reply_response_t wait_for_icmp_reply(int sock_fd, echo_request_t req) {
  unsigned char buffer[BUFFER_LEN*2];
  reply_response_t *res = malloc(sizeof(reply_response_t));

  for (;;) {
    memset(res, 0, sizeof(reply_response_t));
    res->result = -1;

    recv(sock_fd, (char *)&buffer, sizeof(buffer), 0x0);

    // Capture the time we received the message so we can calculate the time
    // elapsed at the end
    struct timeval now;
    if (gettimeofday(&now, NULL) < 0) {
      printf("Error getting the current time\n");
      exit(1);
    }

    unsigned char *packet = buffer;

    if (!ethernet_valid(packet, req))
      continue;
    packet += ETHERNET_LEN;

    if (!ipv4_valid(packet, req, res))
      continue;
    packet += IP_HEADER_LEN;

    if (!icmp_valid(packet, req, res))
      continue;

    long sec_diff_in_usec = (now.tv_sec - req.sent_at.tv_sec) * 1000000;
    long usec_diff        = abs(sec_diff_in_usec + now.tv_usec - req.sent_at.tv_usec);
    res->elapsed_time_in_ms = usec_diff / 1000.0;

    if (res->result == -1)
      res->result = REPLY_SUCCESS;

    break;
  }

  return *res;
}

unsigned char ethernet_valid(unsigned char *packet, echo_request_t req) {
  // Is the packet targeted to us?
  unsigned char i;
  for (i = 0; i < MAC_ADDR_LEN; i++) {
    if (packet[i] != req.local_mac[i])
      return 0;
  }

  // Move past the 2 MAC addresses on the ethernet header
  packet += 2 * MAC_ADDR_LEN;

  // Check if the packet type is an IPv4
  short ether_type;
  CONSUME_BYTES(packet, &ether_type, sizeof(ether_type));
  return ether_type == 8;
}

unsigned char ipv4_valid(unsigned char *packet, echo_request_t req, reply_response_t *res) {
  // Reference to where the IP section starts
  unsigned char *ip_start_ptr = packet;

  // Check if is V4 and has 20 bytes on the header
  char ip_version_and_header_length;
  CONSUME_BYTES(packet, &ip_version_and_header_length, sizeof(ip_version_and_header_length));
  if (ip_version_and_header_length != 0x45)
    return 0;

  // Move forward 1 byte that represents some flags we don't care about
  packet += 1;

  // Total length of the packet
  CONSUME_BYTES(packet, &res->ip_packet_length, sizeof(res->ip_packet_length));
  res->ip_packet_length = ntohs(res->ip_packet_length);

  // Move forward some bytes that represents some stuff we don't care about
  packet += 4; // 2 for packet ID, 2 for flags and offset

  // Extract ttl
  CONSUME_BYTES(packet, &res->ttl, sizeof(res->ttl));

  // Check if the protocol is ICMP
  unsigned char protocol;
  CONSUME_BYTES(packet, &protocol, sizeof(protocol));
  if (protocol != 1)
    return 0;

  // Is the checksum for the IP header correct?
  unsigned short checksum;
  unsigned char *checksum_ptr = packet;
  CONSUME_BYTES(packet, &checksum, sizeof(checksum));
  checksum = ntohs(checksum);
  // Go back to the checksum start and zero it out in order to validate the message
  memset(checksum_ptr, 0, sizeof(checksum));
  // Calculate the checksum with the info we have
  unsigned short calculated_checksum = in_cksum((short unsigned int *)ip_start_ptr, IP_HEADER_LEN);
  calculated_checksum = ntohs(calculated_checksum);

  if (checksum != calculated_checksum)
    return 0;

  // Source IP
  CONSUME_BYTES(packet, &res->source_ip, IP_ADDR_LEN * sizeof(unsigned char));

  // Target IP (us)
  unsigned char target_ip[IP_ADDR_LEN];
  CONSUME_BYTES(packet, target_ip, IP_ADDR_LEN * sizeof(unsigned char));
  // Does the IP match?
  unsigned char i;
  for (i = 0; i < IP_ADDR_LEN; i++) {
    if (target_ip[i] != req.local_ip[i])
      return 0;
  }

  return 1;
}

unsigned char icmp_valid(unsigned char *packet, echo_request_t req, reply_response_t *res) {
  // Reference to where the ICMP section starts
  unsigned char *icmp_start_ptr = packet;

  // Is it an echo reply (or time to live exceeded) with code = 0?
  unsigned char icmp_type;
  CONSUME_BYTES(packet, &icmp_type, sizeof(icmp_type));
  if (icmp_type != 0 && icmp_type != 11)
    return 0;
  unsigned char code;
  CONSUME_BYTES(packet, &code, sizeof(code));
  if (code != 0)
    return 0;

  // Is the checksum for the ICMP header correct?
  unsigned short checksum;
  unsigned char *checksum_ptr = packet;
  CONSUME_BYTES(packet, &checksum, sizeof(checksum));
  checksum = ntohs(checksum);
  // Go back to the checksum start and zero it out in order to validate the message
  memset(checksum_ptr, 0, sizeof(checksum));
  // Calculate the checksum with the info we have
  unsigned short calculated_checksum = in_cksum((short unsigned int *)icmp_start_ptr, res->ip_packet_length - 20);
  calculated_checksum = ntohs(calculated_checksum);
  if (checksum != calculated_checksum)
    return 0;

  // If time to live was exceeded but the checksum is valid, we let the user
  // know about the TTL exceeded, otherwise we discard the packet on the
  // `if` above
  if (icmp_type == 11) {
    res->result = REPLY_TTL_EXCEEDED;
    return 1;
  }

  // Identifier matches?
  unsigned short identifier;
  CONSUME_BYTES(packet, &identifier, sizeof(identifier));
  identifier = ntohs(identifier);
  if (identifier != req.identifier) {
    return 0;
  }

  // Sequence number matches?
  unsigned short sequence_number;
  CONSUME_BYTES(packet, &sequence_number, sizeof(sequence_number));
  sequence_number = ntohs(sequence_number);
  return sequence_number == req.sequence_number;
}
