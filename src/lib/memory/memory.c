#include "heap.h"
#include "memory.h"

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

}

size_t malloc(size_t size) {
    return 0;
}

void free() {

}
