#include "heap.h"
#include "memory.h"

static int heap_validate_table(void* start, void* end, Heap_Table* table) {
    size_t expected_size = (size_t)(end - start);
    size_t expected_blocks = expected_size / KERNEL_HEAP_BLOCK_SIZE;

    if (table->total != expected_blocks) {
        // TODO: use a constant
        return -1;
    }

    return 0;
}

static bool heap_validate_alignment(void* address) {
    return ((unsigned int) address % KERNEL_HEAP_BLOCK_SIZE) == 0;
}

int heap_create_table(Heap_Table* table, void* table_start_address, unsigned int available_bytes) {
    if (available_bytes < KERNEL_HEAP_BLOCK_SIZE) {
        // TODO: use a constant
        return -1;
    }

    table->entries = (HEAP_ENTRY*) table_start_address;
    table->total = available_bytes / KERNEL_HEAP_BLOCK_SIZE;

    return 0;
}

int heap_create(Heap* heap, void* start_addr, Heap_Table* table) {
    void* end_addr = start_addr + table->total;

    if (!heap_validate_alignment(start_addr) || !heap_validate_alignment(end_addr)) {
        // TODO: use a constant
        return -1;
    }

    if (heap_validate_table(start_addr, end_addr, table) < 0) {
        // TODO: use a constant
        return -2;
    }

    memset(heap, 0, sizeof(Heap));
    heap->table = table;
    heap->start_address = start_addr;

    size_t sizeo_of_table_entries = sizeof(HEAP_ENTRY) * table->total;
    memset(table->entries, HEAP_ENTRY_FREE, sizeo_of_table_entries);

    return 0;
}
