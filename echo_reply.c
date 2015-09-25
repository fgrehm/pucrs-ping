#include <stdio.h>
#include <sys/socket.h>
#include "echo_request.h"
#include "echo_reply.h"
#include "constants.h"

reply_response_t wait_for_icmp_reply(int sock_fd, echo_request_t req) {
  printf("Waiting for echo reply packet\n");

  unsigned char buff1[BUFFER_LEN*2];

  for (;;) {
    recv(sock_fd,(char *) &buff1, sizeof(buff1), 0x0);
    printf("  MAC Destino: %x:%x:%x:%x:%x:%x \n", buff1[0],buff1[1],buff1[2],buff1[3],buff1[4],buff1[5]);
    printf("  MAC Origem:  %x:%x:%x:%x:%x:%x \n", buff1[6],buff1[7],buff1[8],buff1[9],buff1[10],buff1[11]);

    // Check if destination MAC address is our interface
    int i, match = 1;
    for (i = 0; i < MAC_ADDR_LEN; i++) {
      printf("Match %x %x\n", buff1[0], req.local_mac[0]);
      if (buff1[i] != req.local_mac[i]) {
        match = 0;
        break;
      }
    }

    if (match) {
      // Verifica se protocolo eh ICMP
      // Verifica se identificador eh igual ao enviado
      // Calcula o q tem q calcular
      break;
    }
  }

  printf("DONE\n");

  reply_response_t res = {
    .success = 1
  };

  return res;
}
