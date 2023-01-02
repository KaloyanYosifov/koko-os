#ifndef HEAP_H
#define HEAP_H

#include "../../config.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define HEAP_ENTRY_FREE 0x00
#define HEAP_ENTRY_TAKEN 0x01

#define HEAP_HAS_NEXT 0x80
#define HEAP_IS_FIRST 0x40

typedef unsigned int HEAP_ENTRY;
typedef uint32_t HEAP_SIZE_TYPE;

typedef struct Heap_Table {
    HEAP_SIZE_TYPE total;
    HEAP_ENTRY* entries;
} Heap_Table;

typedef struct Heap {
    Heap_Table* table;
    void* start_address;
} Heap;

typedef struct Memory_Details {
    void* start_address;
    HEAP_SIZE_TYPE end_block;
    HEAP_SIZE_TYPE start_block;
} Memory_Details;

int heap_create_table(Heap_Table* table, void* table_start_address, size_t available_bytes);
int heap_create(Heap* heap, void* start_address, Heap_Table* table);
void* heap_malloc(Heap* heap, size_t amount_of_bytes);
void heap_free(Heap* heap, void* address);

#endif
