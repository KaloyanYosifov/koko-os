#ifndef MEMORY_H
#define MEMORY_H

#include <stddef.h>

#define MEMORY_HEAP_START_ADDRESS 0x01000000
#define MEMORY_HEAP_TABLE_START_ADDRESS 0x00007E00

void* memset(void* ptr, int c, size_t size);
void memory_init();
void* malloc(size_t amount_of_bytes);
void* zalloc(size_t amount_of_bytes);
void free(void* address);
void memcpy(void* dest, void* src, size_t length);

#endif
