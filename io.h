#ifndef IO_H
#define IO_H

#define CONSUME_BYTES(buffer_ptr, target_ptr, num_bytes) buffer_ptr=consume_bytes(buffer_ptr, target_ptr, num_bytes);

void *consume_bytes(void *buffer_ptr, void *target_ptr, int num_bytes);

#endif
