#ifndef HEAP_H
#define HEAP_H

#include "../../config.h"
#include <stdbool.h>
#include <stddef.h>

#define HEAP_ENTRY_FREE 0x00
#define HEAP_ENTRY_TAKEN 0x01

#define HEAP_HAS_NEXT 0x80
#define HEAP_HAS_IS_FIRST 0x40

typedef unsigned int HEAP_ENTRY;

typedef struct Heap_Table {
    size_t total;
    HEAP_ENTRY* entries;
} Heap_Table;

typedef struct Heap {
    Heap_Table* table;
    void* start_address;
} Heap;

int heap_create_table(Heap_Table* table, void* table_start_address, unsigned int available_bytes);
int heap_create(Heap* heap, void* ptr, Heap_Table* table);

#endif
