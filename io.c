#include <string.h>
#include "io.h"

void *consume_bytes(void *buffer_ptr, void *target_ptr, int num_bytes) {
  memcpy(target_ptr, buffer_ptr, num_bytes);
  return buffer_ptr + num_bytes;
}

void *append_bytes(void *buffer_ptr, void *src_ptr, int num_bytes) {
  memcpy(buffer_ptr, src_ptr, num_bytes);
  return buffer_ptr + num_bytes;
}

void *append_byte(void *buffer_ptr, unsigned char byte) {
  memset(buffer_ptr, byte, 1);
  return buffer_ptr + 1;
}

void *append_short(void *buffer_ptr, unsigned short short_num) {
  memset(buffer_ptr, short_num, 2);
  return buffer_ptr + 2;
}
