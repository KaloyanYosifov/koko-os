#include "memory.h"
#include <stdint.h>

void* memset(void* ptr, int c, size_t size) {
    uint32_t* c_ptr = (uint32_t*) ptr;

    for (size_t i = 0; i < size / sizeof(uint32_t); i++) {
        c_ptr[i] = c;
    }

    return ptr;
}
