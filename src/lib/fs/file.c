#include "file.h"

#include "./fat16/fat16.h"
#include "../../kernel.h"
#include "../memory/memory.h"
#include "../../config.h"
#include "../../errors.h"

File_System* file_systems[KERNEL_MAX_FILESYSTEMS];
File_Descriptor* file_descriptors[KERNEL_MAX_FILE_DESCRIPTORS];

static int8_t fs_get_free_file_system() {
    for (uint8_t i = 0; i < KERNEL_MAX_FILESYSTEMS; i++) {
        if (file_systems[i] == NULL) {
            return i;
        }
    }

    return -1;
}

void fs_insert_filesystem(File_System* fs) {
    if (fs == NULL) {
        panic("No file system entered as argument!");
    }

    int8_t slot = fs_get_free_file_system();

    if (slot < 0) {
        panic("No free slot for file system available!");
    }

    file_systems[slot] = fs;
}

void fs_init() {
    memset(file_systems, 0, sizeof(file_systems));
    memset(file_descriptors, 0, sizeof(file_descriptors));

    // load static file systems here
    fs_insert_filesystem(fat16_init());
}

File_System* fs_resolve(Disk* disk) {
    for (uint8_t i = 0; i < KERNEL_MAX_FILESYSTEMS; i++) {
        if (file_systems[i] != NULL && file_systems[i]->resolve(disk) == OK) {
            return file_systems[i];
        }
    }

    return NULL;
}

FD_INDEX fs_open(char* filename, FILE_MODE mode) {
    return NOT_IMPLEMENTED;
}

static File_Descriptor* fs_new_file_descriptor() {
    for (unsigned int i = 0; i < KERNEL_MAX_FILE_DESCRIPTORS; i++) {
        if (file_descriptors[i] != NULL) {
            continue;
        }

        File_Descriptor* fd = zalloc(sizeof(File_Descriptor));
        // we always start from 1 instead of 0 for file descriptor index
        fd->index = i + 1;

        file_descriptors[i] = fd;

        return fd;
    }

    return NULL;
}

static File_Descriptor* fs_find_file_descriptor(FD_INDEX index) {
    if (index <= 0 || index >= KERNEL_MAX_FILE_DESCRIPTORS) {
        return NULL;
    }

    return file_descriptors[index - 1];
}
