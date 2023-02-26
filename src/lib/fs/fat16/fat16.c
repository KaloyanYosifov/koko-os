#include "fat16.h"

#include "../../string.h"
#include "../../../config.h"
#include "../../../errors.h"

#include <stddef.h>

typedef void* (*FS_OPEN_FUNCTION)(Disk* disk, Path_Part* path, FILE_MODE mode);
typedef int (*FS_RESOLVE_FUNCTION)(Disk* disk);

void* fat16_open(Disk* disk, Path_Part* path, FILE_MODE mode) {
    return NULL;
}

int fat16_resolve(Disk* disk) {
    if (disk->type == DISK_REAL_DISK_TYPE) {
        return OK;
    }

    return INVALID_ARGUMENT;
}

File_System fat16_file_system = {
    .resolve = fat16_resolve,
    .open = fat16_open
};

File_System* fat16_init() {
    str_ref_copy(fat16_file_system.name, "FAT 16");

    return &fat16_file_system;
}
