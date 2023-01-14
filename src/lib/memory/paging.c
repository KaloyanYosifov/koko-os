#include "paging.h"
#include "memory.h"

#include "../../config.h"
#include "../../errors.h"

extern void paging_enable_paging();
extern void paging_set_global_page_directory(PAGE_DIRECTORY* directory);

Paging_Chunk* paging_create_chunk(uint8_t flags) {
    PAGE_DIRECTORY* directory = malloc(sizeof(PAGE_DIRECTORY) * PAGING_PAGE_ENTRIES);
    unsigned int offset = 0;

    for (uint16_t i = 0; i < PAGING_PAGE_ENTRIES; i++) {
        PAGE_TABLE_ENTRY* entry = malloc(sizeof(PAGE_TABLE_ENTRY) * PAGING_PAGE_ENTRIES);

        for (uint16_t j = 0; j < PAGING_PAGE_ENTRIES; j++) {
            entry[j] = (offset + (j * PAGING_PAGE_SIZE)) | flags;
        }
        // we offset to the next page entry start point
        offset += PAGING_PAGE_ENTRIES * PAGING_PAGE_SIZE;

        directory[i] = ((PAGE_TABLE_ENTRY) entry) | flags | PAGING_PAGE_IS_WRITABLE;
    }

    Paging_Chunk* chunk = zalloc(sizeof(PAGE_DIRECTORY));
    chunk->directory = directory;

    return chunk;
}

void paging_switch_directory(PAGE_DIRECTORY* directory) {
    paging_set_global_page_directory(directory);
}

bool paging_is_aligned(void* address) {
    return ((uint32_t) address % PAGING_PAGE_SIZE) == 0;
}

Paging_Index_Info paging_get_indexes(void* virtual_address) {
    Paging_Index_Info info;
    info.error_code = OK;
    info.table_index = 0;
    info.directory_index = 0;

    if (!paging_is_aligned(virtual_address)) {
        info.error_code = PAGING_MEMORY_NOT_ALIGNED;

        return info;
    }

    info.directory_index = ((uint32_t) virtual_address) / (PAGING_PAGE_ENTRIES * PAGING_PAGE_SIZE);
    info.table_index = (((uint32_t) virtual_address) % (PAGING_PAGE_ENTRIES * PAGING_PAGE_SIZE)) / PAGING_PAGE_SIZE;

    return info;
}

int paging_set(PAGE_DIRECTORY* directory, void* virtual_address, PAGE_TABLE_ENTRY flags) {
    if (!paging_is_aligned(virtual_address)) {
        return PAGING_MEMORY_NOT_ALIGNED;
    }

    Paging_Index_Info info = paging_get_indexes(virtual_address);

    if (info.error_code != OK) {
        return info.error_code;
    }

    PAGE_TABLE_ENTRY entry = directory[info.directory_index];
    uint32_t* table = (uint32_t*) (entry & 0xfffff000);

    table[info.table_index] = flags;

    return OK;
}
