#ifndef ECHO_REQUEST_H
#define ECHO_REQUEST_H

// A struct that help us pass less arguments around and generate the report
// at the end
typedef struct {
  unsigned short identifier;
  unsigned short sequence_number;
  struct timeval sent_at;
  unsigned char *local_mac;
  unsigned char *local_ip;
  char *raw_packet;
} echo_request_t;

echo_request_t prepare_echo_request(unsigned short identifier, unsigned char *local_ip, unsigned char *local_mac, unsigned char *dest_ip, unsigned char *dest_mac);

#endif
