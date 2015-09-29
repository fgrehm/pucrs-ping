#ifndef ECHO_REPLY_H
#define ECHO_REPLY_H

#include "constants.h"

#define REPLY_SUCCESS 1
#define REPLY_TTL_EXCEEDED 2
#define REPLY_TIMEOUT 3

typedef struct {
  int result;
  unsigned char ttl;
  unsigned short ip_packet_length;
  float elapsed_time_in_ms;
  unsigned char source_ip[IP_ADDR_LEN];
} reply_response_t;

reply_response_t wait_for_icmp_reply(int sock_fd, echo_request_t req);

#endif
