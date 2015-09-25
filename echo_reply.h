#ifndef ECHO_REPLY_H
#define ECHO_REPLY_H

int wait_for_icmp_reply(int sock_fd, char *local_ip, char *local_mac_str, char *dest_ip, char *dest_mac_str);

#endif
