#ifndef ECHO_REQUEST_H
#define ECHO_REQUEST_H

int send_echo_request_packet(int sock_fd, char *local_ip, unsigned char *local_mac, char *dest_ip, unsigned char *dest_mac);

#endif
