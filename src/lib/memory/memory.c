#include "heap.h"
#include "memory.h"

#include <stdint.h>

struct heap heap;
struct heap_table heap_table;

void* memset(void* ptr, int c, size_t size) {
    uint32_t* c_ptr = (uint32_t*) ptr;

    for (size_t i = 0; i < size / sizeof(uint32_t); i++) {
        c_ptr[i] = c;
    }

    return ptr;
}

void memory_init() {
    memset(&heap, 0, sizeof(heap));
    memset(&heap_table, 0, sizeof(heap_table));

    heap.start_address = (void*) MEMORY_HEAP_START_ADDRESS;
}
