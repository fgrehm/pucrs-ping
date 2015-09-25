#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include "echo_request.h"
#include "echo_reply.h"
#include "constants.h"

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

    unsigned char *bufferptr = buffer;

    // Check if the packet type is an IPv4
    short ether_type;
    bufferptr += 2 * MAC_ADDR_LEN;
    memcpy(&ether_type, bufferptr, sizeof(ether_type));
    if (ether_type != 8) // IP
      continue;
    // Check if is V4 and has 20 bytes on the header
    bufferptr += 1;
    short ip_version_and_header_length;
    memcpy(&ip_version_and_header_length, bufferptr, sizeof(ip_version_and_header_length));
    if (ip_version_and_header_length != 0x4500)
      continue;
    bufferptr += 1;

    // Extract ttl
    bufferptr += 8;
    memcpy(&ttl, bufferptr, sizeof(ttl));
    bufferptr += 1;

    // Check if the protocol is ICMP
    unsigned char protocol;
    memcpy(&protocol, bufferptr, sizeof(protocol));
    if (protocol != 1)
      continue;

    // Check if the reply is for us

    // TODO: Calculate time elapsed

    if (match) {
      printf("  - TODO: Terminar parse do echo reply\n");
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
