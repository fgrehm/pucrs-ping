#ifndef ECHO_REPLY_H
#define ECHO_REPLY_H

#include "constants.h"

typedef struct {
  char success;
  char timed_out;
  char ttl_exceeded;
  char ttl;
  unsigned char *source_ip;
  float elapsed_time_in_ms;
} reply_response_t;

reply_response_t wait_for_icmp_reply(int sock_fd, echo_request_t req);

#endif
