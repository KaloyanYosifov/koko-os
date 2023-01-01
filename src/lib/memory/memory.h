#ifndef MEMORY_H
#define MEMORY_H

#include <stddef.h>

#define MEMORY_HEAP_START_ADDRESS 0x01000000

void* memset(void* ptr, int c, size_t size);
void memory_init();

#endif
