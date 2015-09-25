#ifndef ECHO_REPLY_H
#define ECHO_REPLY_H

int wait_for_icmp_reply(int sock_fd, unsigned char *local_ip, unsigned char *local_mac_str, unsigned char *dest_ip, unsigned char *dest_mac_str);

#endif
