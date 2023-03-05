#include "disk.h"
#include "io.h"
#include "../../config.h"
#include "../../errors.h"
#include "../memory/memory.h"

#include <stddef.h>

Disk primary_disk;

Disk_Sector_Info disk_read_sector(unsigned int lba, uint8_t total_sectors_to_read) {
    Disk_Sector_Info info;

    memset(&info, (int) NULL, sizeof(Disk_Sector_Info));

    if (total_sectors_to_read == 0) {
        info.error_code = INVALID_ARGUMENT;

        return info;
    }

    info.error_code = OK;
    info.buffer = zalloc(total_sectors_to_read * DISK_SECTOR_BYTES);

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

        uint16_t* ptr = info.buffer;

        for (unsigned int i = 0; i < DISK_SECTOR_WORDS; i++) {
            *ptr = insw(0x01F0);
            ptr++;
        }
    }

    return info;
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

Disk_Sector_Info disk_read_block(Disk* disk, unsigned int lba, uint8_t total_sectors_to_read) {
    Disk_Sector_Info info;

    memset(&info, (int) NULL, sizeof(Disk_Sector_Info));

    // TODO: support multiple disks
    if (disk != &primary_disk) {
        info.error_code = DISK_INVALID_DISK;

        return info;
    }

    return disk_read_sector(lba, total_sectors_to_read);
}
