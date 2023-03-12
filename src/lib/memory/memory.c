#include "heap.h"
#include "memory.h"
#include "../terminal.h"
#include "../../config.h"
#include "../../kernel.h"

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
    int status = heap_create_table(&heap_table, (void*) MEMORY_HEAP_TABLE_START_ADDRESS, KERNEL_MAX_HEAP_MEMORY);

    if (status != OK) {
        panic("Failed to initialize heap table!");
    }

    status = heap_create(&heap, (void*) MEMORY_HEAP_START_ADDRESS, &heap_table);

    if (status != OK) {
        panic("Failed to initialize heap!");
    }
}

void* malloc(size_t amount_of_bytes) {
    return heap_malloc(&heap, amount_of_bytes);
}

void* zalloc(size_t amount_of_bytes) {
    void* ptr = heap_malloc(&heap, amount_of_bytes);

    memset(ptr, 0x00, amount_of_bytes);

    return ptr;
}

void memcpy(void* dest, void* src, size_t length) {
    uint8_t* d_temp = dest;
    uint8_t* s_temp = src;

    while (length-- > 0) {
        *d_temp = *s_temp;

        d_temp++;
        s_temp++;
    }
}

void free(void* address) {
    heap_free(&heap, address);
}
