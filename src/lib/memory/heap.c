#include "heap.h"
#include "memory.h"
#include "../terminal.h"
#include "../../kernel.h"
#include "../../config.h"
#include "../../errors.h"

static int heap_validate_table(void* start, void* end, Heap_Table* table) {
    HEAP_SIZE_TYPE expected_size = (HEAP_SIZE_TYPE)(end - start);
    HEAP_SIZE_TYPE expected_blocks = expected_size / KERNEL_HEAP_BLOCK_SIZE;

    if (table->total != expected_blocks) {
        return HEAP_TABLE_LOW_BYTES_AVAILABLE;
    }

    return OK;
}

static bool heap_validate_alignment(void* address) {
    return ((unsigned int) address % 4096) == 0;
}

int heap_create_table(Heap_Table* table, void* table_start_address, size_t available_bytes) {
    if (available_bytes < KERNEL_HEAP_BLOCK_SIZE) {
        return HEAP_AVAILABLE_MEMORY_LESS_THAN_BLOCK;
    }

    table->entries = (HEAP_ENTRY*) table_start_address;
    table->total = available_bytes / KERNEL_HEAP_BLOCK_SIZE;

    return OK;
}

int heap_create(Heap* heap, void* start_address, Heap_Table* table) {
    void* end_addr = start_address + (table->total * KERNEL_HEAP_BLOCK_SIZE);

    if (!heap_validate_alignment(start_address) || !heap_validate_alignment(end_addr)) {
        return HEAP_MEMORY_NOT_ALIGNED;
    }


    int validation_result = heap_validate_table(start_address, end_addr, table);
    if (validation_result != OK) {
        return validation_result;
    }

    memset(heap, 0, sizeof(Heap));
    heap->table = table;
    heap->start_address = start_address;

    size_t sizeo_of_table_entries = sizeof(HEAP_ENTRY) * table->total;
    memset(table->entries, HEAP_ENTRY_FREE, sizeo_of_table_entries);

    return OK;
}

static HEAP_SIZE_TYPE heap_align_amount_of_bytes_requested(HEAP_SIZE_TYPE amount_of_bytes) {
    if (amount_of_bytes % KERNEL_HEAP_BLOCK_SIZE == 0) {
        return amount_of_bytes;
    }

    amount_of_bytes = amount_of_bytes - (amount_of_bytes % KERNEL_HEAP_BLOCK_SIZE);
    amount_of_bytes += KERNEL_HEAP_BLOCK_SIZE;

    return amount_of_bytes;
}

static bool heap_entry_is_taken(HEAP_ENTRY entry) {
    return (entry & HEAP_ENTRY_TAKEN) == 1;
}

static bool heap_entry_is_free(HEAP_ENTRY entry) {
    return !heap_entry_is_taken(entry);
}

static bool heap_entry_has_next(HEAP_ENTRY entry) {
    return (entry & HEAP_HAS_NEXT) > 0;
}

static bool heap_entry_is_first(HEAP_ENTRY entry) {
    return (entry & HEAP_IS_FIRST) > 0;
}

static void* heap_get_address_from_block(Heap* heap, HEAP_SIZE_TYPE start_block) {
    return heap->start_address + (start_block * KERNEL_HEAP_BLOCK_SIZE);
}

static Memory_Details heap_request_memory(Heap* heap, HEAP_SIZE_TYPE amount_of_bytes) {
    if (amount_of_bytes <= 0) {
        Memory_Details details;
        details.start_address = NULL;
        details.start_block = 0;
        details.end_block = 0;

        return details;
    }

    HEAP_SIZE_TYPE required_blocks = amount_of_bytes / KERNEL_HEAP_BLOCK_SIZE;
    HEAP_SIZE_TYPE current_block = 0;
    HEAP_SIZE_TYPE block_count = 0;

    for (HEAP_SIZE_TYPE i = 0; i < heap->table->total; i++) {
       HEAP_ENTRY entry = heap->table->entries[i];

       if (heap_entry_is_free(entry)) {
           // set the current block when we have found a free block initially
           if (block_count == 0) {
                current_block = i;
           }

            block_count += 1;
       } else if (heap_entry_is_taken(entry)) {
            current_block = 0;
            block_count = 0;
       }

       if (block_count == required_blocks) {
            break;
       }
    }

    if (block_count != required_blocks) {
        panic("Not enough memory");
    }

    Memory_Details details;
    details.start_address = heap_get_address_from_block(heap, current_block);
    details.start_block = current_block;
    // get index of end block
    details.end_block = current_block + block_count - 1;

    return details;
}

void* heap_malloc(Heap* heap, size_t amount_of_bytes) {
    amount_of_bytes = heap_align_amount_of_bytes_requested(amount_of_bytes);
    Memory_Details details = heap_request_memory(heap, amount_of_bytes);

    for (HEAP_SIZE_TYPE i = details.start_block; i <= details.end_block; i++) {
        HEAP_ENTRY entry = 0;

        entry |= HEAP_ENTRY_TAKEN;

        if (i == details.start_block) {
            entry |= HEAP_IS_FIRST;
        } else if (i != details.end_block) {
            entry |= HEAP_HAS_NEXT;
        }

        heap->table->entries[i] = entry;
    }

    return details.start_address;
}

void heap_free(Heap* heap, void* address) {
    // if we have our start address is bigger than the address passed
    // it should be an invalid case, as the address cannot be less than the starting point
    if (heap->start_address > address) {
        panic("Invalid address is going to be freed!");
    }

    HEAP_SIZE_TYPE start_block = 0;

    // TODO: handle case where invalid address that is not aligned to our block size is passed

    if (heap->start_address != address) {
        start_block = (address - heap->start_address) / KERNEL_HEAP_BLOCK_SIZE;
    }

    Heap_Table *table = heap->table;
    HEAP_ENTRY* entry = &table->entries[start_block];

    if (!heap_entry_is_first(*entry)) {
        panic("Address is not the beginning of an address block!");
    }

    HEAP_SIZE_TYPE current_block = start_block;

    do {
        *entry = HEAP_ENTRY_FREE;
        void* entry_address = heap_get_address_from_block(heap, start_block);
        memset(entry_address, 0, KERNEL_HEAP_BLOCK_SIZE);

        entry = &table->entries[++current_block];
    } while(heap_entry_has_next(*entry));
}
