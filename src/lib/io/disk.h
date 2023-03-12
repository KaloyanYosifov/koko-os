#ifndef DISK_H
#define DISK_H

#include <stdint.h>
#include "../fs/file.h"

#define DISK_SECTOR_BYTES 512
#define DISK_SECTOR_WORDS DISK_SECTOR_BYTES / 2
#define DISK_REAL_DISK_TYPE 0

typedef uint8_t DISK_TYPE;

typedef struct disk {
    uint8_t id;
    DISK_TYPE type;
    unsigned int sector_size;
    struct file_system* fs;

    // private data of the filesystem
    void* fs_private;
} Disk;

void disk_init();
Disk* disk_get(DISK_TYPE type);
int disk_read_block(Disk* disk, char* bufffer, unsigned int lba, uint8_t total);

#endif
