#include "disk.h"
#include "io.h"
#include "../../config.h"
#include "../../errors.h"
#include "../memory/memory.h"

#include <stddef.h>

Disk_Sector_Info disk_read_sector(unsigned int lba, uint8_t total) {
    Disk_Sector_Info info;
    info.buffer = NULL;

    if (total == 0) {
        info.error_code = INVALID_ARGUMENT;

        return info;
    }

    info.error_code = OK;
    info.buffer = zalloc(total * DISK_SECTOR_BYTES);

    outb(0x01F6, (lba >> 24) | 0xE0);
    outb(0x01F2, total);
    outb(0x01F3, (lba & 0xff));
    outb(0x01F4, (lba >> 8));
    outb(0x01F5, (lba >> 16));
    outb(0x01F7, 0x20);

    for (unsigned int i = 0; i < total; i++) {
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
