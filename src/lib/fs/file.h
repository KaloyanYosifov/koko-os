#ifndef FILE_H
#define FILE_H

#include <stdint.h>
#include "path_parser.h"
#include "../io/disk.h"

typedef struct disk Disk;

typedef uint16_t FD_INDEX;

typedef enum seek_mode {
    SEEK_MODE_SET,
    SEEK_MODE_CUR,
    SEEK_MODE_END
} SEEK_MODE;

typedef enum file_mode {
    FIEL_MODE_READ,
    FIEL_MODE_WRITE,
    FIEL_MODE_APPEND,
    FIEL_MODE_INVALID
} FILE_MODE;

typedef void* (*FS_OPEN_FUNCTION)(Disk* disk, Path_Part* path, FILE_MODE mode);
typedef int (*FS_RESOLVE_FUNCTION)(Disk* disk);

typedef struct file_system {
    FS_OPEN_FUNCTION open;
    FS_RESOLVE_FUNCTION resolve;
    char name[20];
} File_System;

typedef struct file_descriptor {
    FD_INDEX index;
    File_System* fs;

    // Used by the underline filesystem
    void* private_data;
} File_Descriptor;

void fs_init();
FD_INDEX fs_open(char* filename, FILE_MODE mode);
void fs_insert_filesystem(File_System* fs);
File_System* fs_resolve(Disk* disk);

#endif
