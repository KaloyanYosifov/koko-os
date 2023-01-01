#ifndef HEAP_H
#define HEAP_H

#include "../../config.h"
#include <stddef.h>

#define HEAP_ENTRY_FREE 0x00
#define HEAP_ENTRY_TAKEN 0x01

#define HEAP_HAS_NEXT 0x80
#define HEAP_HAS_IS_FIRST 0x40

typedef unsigned int HEAP_ENTRY;

struct heap_table {
    HEAP_ENTRY* entries;
    size_t total_used;
};

struct heap {
    struct heap_table* table;
    void* start_address;
};

void heap_create();

#endif
