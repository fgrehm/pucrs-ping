#include <string.h>
#include "io.h"

void *consume_bytes(void *buffer_ptr, void *target_ptr, int num_bytes) {
  memcpy(target_ptr, buffer_ptr, num_bytes);
  return buffer_ptr + num_bytes;
}
