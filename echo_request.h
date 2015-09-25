#ifndef ECHO_REQUEST_H
#define ECHO_REQUEST_H

typedef struct {
  unsigned short sequence_number;
  int            timestamp;
  char           *raw_packet;
} echo_request_t;

echo_request_t prepare_echo_request(unsigned short identifier, unsigned char *local_ip, unsigned char *local_mac, unsigned char *dest_ip, unsigned char *dest_mac);

#endif
