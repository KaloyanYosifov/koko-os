#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>

// flags
#define PAGING_PAGE_IS_PRESENT          0x1
#define PAGING_PAGE_IS_WRITABLE         0x2
#define PAGING_PAGE_ALLOW_ACCESS_TO_ALL 0x4
#define PAGING_PAGE_WRITE_THROUGH_CACHE 0x8
#define PAGING_PAGE_DISABLE_CACHING     0x10

// config
#define PAGING_PAGE_ENTRIES 1024
#define PAGING_PAGE_SIZE 4096

typedef uint32_t PAGE_DIRECTORY;
typedef uint32_t PAGE_TABLE_ENTRY;

// paging_chunk for 32 bit mode
// paging_long_chunk for 64 bit
typedef struct paging_chunk {
    PAGE_DIRECTORY* directory;
} Paging_Chunk;

void paging_enable_paging();

Paging_Chunk* paging_create_chunk(uint8_t flags);
void paging_switch_directory(PAGE_DIRECTORY* directory);

#endif
