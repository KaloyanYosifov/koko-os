#include "paging.h"
#include "memory.h"

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
