#ifndef ECHO_REQUEST_H
#define ECHO_REQUEST_H

int send_echo_request_packet(int sock_fd, unsigned char *local_ip, unsigned char *local_mac, unsigned char *dest_ip, unsigned char *dest_mac);

#endif
