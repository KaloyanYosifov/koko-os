#include "heap.h"
#include "memory.h"
#include "../terminal.h"

#include <stdint.h>

Heap heap;
Heap_Table heap_table;

void* memset(void* ptr, int c, size_t size) {
    uint32_t* c_ptr = (uint32_t*) ptr;

    for (size_t i = 0; i < size / sizeof(uint32_t); i++) {
        c_ptr[i] = c;
    }

    return ptr;
}

void memory_init() {
    heap_create_table(&heap_table, (void*) MEMORY_HEAP_TABLE_START_ADDRESS, KERNEL_MAX_HEAP_MEMORY);
    heap_create(&heap, (void*) MEMORY_HEAP_START_ADDRESS, &heap_table);

    println("test");
}

void* malloc(size_t amount_of_bytes) {
    return heap_malloc(&heap, amount_of_bytes);
}

void free() {

}
