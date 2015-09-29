#ifndef IO_H
#define IO_H

#define CONSUME_BYTES(buffer_ptr, target_ptr, num_bytes) buffer_ptr=consume_bytes(buffer_ptr, target_ptr, num_bytes);
void *consume_bytes(void *buffer_ptr, void *target_ptr, int num_bytes);

#define APPEND_BYTES(buffer_ptr, src_ptr, num_bytes) buffer_ptr=append_bytes(buffer_ptr, src_ptr, num_bytes);
void *append_bytes(void *buffer_ptr, void *src_ptr, int num_bytes);

#define APPEND_BYTE(buffer_ptr, byte) buffer_ptr=append_byte(buffer_ptr, byte);
void *append_byte(void *buffer_ptr, unsigned char byte);

#define APPEND_SHORT(buffer_ptr, short_num) buffer_ptr=append_short(buffer_ptr, short_num);
void *append_short(void *buffer_ptr, unsigned short short_num);

#endif
