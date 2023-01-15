#ifndef DISK_H
#define DISK_H

#include <stdint.h>

#define DISK_SECTOR_BYTES 512
#define DISK_SECTOR_WORDS DISK_SECTOR_BYTES / 2

typedef struct disk_sector_info {
   int16_t error_code;
   uint16_t* buffer;
} Disk_Sector_Info;

Disk_Sector_Info disk_read_sector(unsigned int lba, uint8_t total);

#endif
