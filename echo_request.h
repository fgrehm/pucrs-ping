#ifndef ECHO_REQUEST_H
#define ECHO_REQUEST_H

int send_echo_request_packet(int sock_fd, char *local_ip, char *local_mac_str, char *dest_ip, char *dest_mac_str);

#endif
