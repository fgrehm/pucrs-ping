#include <stdio.h>
#include <sys/socket.h>
#include "constants.h"

void wait_for_icmp_reply(int sock_fd, char *local_mac, char *local_ip, char *dest_mac, char *dest_ip) {
  printf("Waiting for ICMP packets\n");

  unsigned char buff1[BUFFER_LEN];

  // TODO: Timeout if time exceeded (10 seconds maybe?)
  recv(sock_fd,(char *) &buff1, sizeof(buff1), 0x0);
  printf("MAC Destino: %x:%x:%x:%x:%x:%x \n", buff1[0],buff1[1],buff1[2],buff1[3],buff1[4],buff1[5]);
  printf("MAC Origem:  %x:%x:%x:%x:%x:%x \n\n", buff1[6],buff1[7],buff1[8],buff1[9],buff1[10],buff1[11]);

  // Verifica se MAC destino eh o mac de origem
  // Verifica se protocolo eh ICMP
  // Verifica se identificador eh igual ao enviado
  // Calcula o q tem q calcular
}
