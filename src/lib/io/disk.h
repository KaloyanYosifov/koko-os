#ifndef DISK_H
#define DISK_H

#include <stdint.h>
#include "../fs/file.h"

#define DISK_SECTOR_BYTES 512
#define DISK_SECTOR_WORDS DISK_SECTOR_BYTES / 2
#define DISK_REAL_DISK_TYPE 0

typedef uint32_t DISK_TYPE;

typedef struct disk {
    DISK_TYPE type;
    unsigned int sector_size;
    struct file_system* fs;
} Disk;

typedef struct disk_sector_info {
   int16_t error_code;
   uint16_t* buffer;
} Disk_Sector_Info;

void disk_init();
Disk* disk_get(DISK_TYPE type);
Disk_Sector_Info disk_read_block(Disk* disk, unsigned int lba, uint8_t total);

#endif
