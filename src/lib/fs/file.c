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

static int fs_free_file_descriptor(FD_INDEX fd) {
    if (fd <= 0) {
        return FS_INVALID_FILE_DESCRIPTOR;
    }

    File_Descriptor* desc = file_descriptors[fd - 1];

    if (desc == NULL) {
        return INVALID_ARGUMENT;
    }

    free(desc);
    file_descriptors[fd - 1] = NULL;

    return OK;
}

static File_Descriptor* fs_find_file_descriptor(FD_INDEX index) {
    if (index <= 0 || index >= KERNEL_MAX_FILE_DESCRIPTORS) {
        return NULL;
    }

    return file_descriptors[index - 1];
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
    // get path and validate if it is correct. Not 0:/
    Path_Root* root = path_parser_parse_path(filename);

    if (!root || !root->part || !root->part->name) {
        return NULL_FILE_DESCRIPTOR;
    }

    // get disk and validate if it is correct
    // or check if disk has a filesystem
    Disk* disk = disk_get(root->driver_no);

    if (!disk || !disk->fs) {
        return NULL_FILE_DESCRIPTOR;
    }

    // call fs open and check for error
    void* file_data = disk->fs->open(disk, root->part, mode);

    if (IS_ERROR(ERROR_I(file_data))) {
        return NULL_FILE_DESCRIPTOR;
    }

    // create a file descriptor and check if it is ok
    File_Descriptor* fd = fs_new_file_descriptor();

    if (!fd) {
        return NULL_FILE_DESCRIPTOR;
    }

    // assign relevant values to file descriptor
    fd->fs = disk->fs;
    fd->disk = disk;
    fd->private_data = file_data;

    return fd->index;
}

int fs_read (void* ptr, FD_INDEX fd, uint32_t size, uint32_t nmemb) {
    if (size == 0 || nmemb == 0 || fd <= 0) {
        return INVALID_ARGUMENT;
    }

    File_Descriptor* descriptor = fs_find_file_descriptor(fd);

    if (!descriptor) {
        return FS_INVALID_FILE_DESCRIPTOR;
    }

    descriptor->fs->read(descriptor->disk, (char*) ptr, descriptor->private_data, size, nmemb);

    return OK;
}

int fs_write (void* in, FD_INDEX fd, uint32_t size) {
    if (size == 0 || fd <= 0) {
        return INVALID_ARGUMENT;
    }

    File_Descriptor* descriptor = fs_find_file_descriptor(fd);

    if (!descriptor) {
        return FS_INVALID_FILE_DESCRIPTOR;
    }

    descriptor->fs->write(descriptor->disk, (char*) in, descriptor->private_data, size);

    return OK;
}

int fs_seek(FD_INDEX fd, uint32_t offset, SEEK_MODE mode) {
    File_Descriptor* descriptor = fs_find_file_descriptor(fd);

    if (!descriptor) {
        return FS_INVALID_FILE_DESCRIPTOR;
    }

    return descriptor->disk->fs->seek(descriptor->private_data, offset, mode);
}

int fs_stat(FD_INDEX fd, File_Stat* stat) {
    File_Descriptor* descriptor = fs_find_file_descriptor(fd);

    if (!descriptor) {
        return FS_INVALID_FILE_DESCRIPTOR;
    }

    return descriptor->disk->fs->stat(descriptor->disk, descriptor->private_data, stat);
}

int fs_close(FD_INDEX fd) {
    File_Descriptor* descriptor = fs_find_file_descriptor(fd);

    if (!descriptor) {
        return FS_INVALID_FILE_DESCRIPTOR;
    }

    if (descriptor->disk->fs->close(descriptor->private_data) != OK) {
        return SYSTEM_FAIL;
    }

    fs_free_file_descriptor(fd);

    return OK;
}
