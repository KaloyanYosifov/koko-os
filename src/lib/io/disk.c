#include "disk.h"
#include "io.h"
#include "../../config.h"
#include "../../errors.h"
#include "../memory/memory.h"

#include <stddef.h>

Disk primary_disk;

int disk_read_sector(char* buffer, unsigned int lba, uint8_t total_sectors_to_read) {
    if (total_sectors_to_read == 0) {
        return INVALID_ARGUMENT;
    }

    outb(0x01F6, (lba >> 24) | 0xE0);
    outb(0x01F2, total_sectors_to_read);
    outb(0x01F3, (lba & 0xff));
    outb(0x01F4, (lba >> 8));
    outb(0x01F5, (lba >> 16));
    outb(0x01F7, 0x20);

    for (unsigned int i = 0; i < total_sectors_to_read; i++) {
        uint8_t c = insb(0x01F7);

        // loop until sector buffer is ready
        while (!(c & 0x08)) {
            c = insb(0x01F7);
        }

        uint16_t* ptr = (uint16_t*) buffer;

        for (unsigned int i = 0; i < DISK_SECTOR_WORDS; i++) {
            *ptr = insw(0x01F0);
            ptr++;
        }
    }

    return OK;
}

void disk_init() {
    primary_disk.id = 0;
    primary_disk.type = DISK_REAL_DISK_TYPE;
    primary_disk.sector_size = KERNEL_DEFAULT_DISK_SECTOR_SIZE;
    primary_disk.fs = fs_resolve(&primary_disk);
}

Disk* disk_get(DISK_TYPE type) {
    // TODO: support multiple disks
    if (type != DISK_REAL_DISK_TYPE) {
       return NULL;
    }

    return &primary_disk;
}

int disk_read_block(Disk* disk, char* buffer, unsigned int lba, uint8_t total_sectors_to_read) {
    if (disk != &primary_disk) {
        return DISK_INVALID_DISK;
    }

    return disk_read_sector(buffer, lba, total_sectors_to_read);
}
