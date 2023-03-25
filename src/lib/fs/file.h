#ifndef FILE_H
#define FILE_H

#include <stdint.h>
#include "path_parser.h"
#include "../io/disk.h"

#define NULL_FILE_DESCRIPTOR 0

typedef struct disk Disk;

typedef uint16_t FD_INDEX;

typedef enum seek_mode {
    SEEK_MODE_SET,
    SEEK_MODE_CUR,
    SEEK_MODE_END
} SEEK_MODE;

typedef enum file_mode {
    FILE_MODE_READ,
    FILE_MODE_WRITE,
    FILE_MODE_APPEND,
} FILE_MODE;

typedef enum file_stat_flags {
    FILE_STAT_READ_ONLY = 0x01
} FILE_STAT_FLAGS;

typedef struct file_stat {
    FILE_STAT_FLAGS flags;
    uint32_t size;
} File_Stat;

typedef int (*FS_RESOLVE_FUNCTION)(Disk* disk);
typedef int (*FS_CLOSE_FUNCTION)(void* private_data);
typedef void* (*FS_OPEN_FUNCTION)(Disk* disk, Path_Part* path, FILE_MODE mode);
typedef int (*FS_STAT_FUNCTION)(Disk* disk, void* private_data, File_Stat* stat);
typedef int (*FS_SEEK_FUNCTION)(void* private_data, uint32_t offset, SEEK_MODE mode);
typedef int (*FS_READ_FUNCTION)(Disk* disk, char* out, void* private_data, uint32_t size, uint32_t nmemb);

typedef struct file_system {
    FS_OPEN_FUNCTION open;
    FS_READ_FUNCTION read;
    FS_SEEK_FUNCTION seek;
    FS_STAT_FUNCTION stat;
    FS_CLOSE_FUNCTION close;
    FS_RESOLVE_FUNCTION resolve;
    char name[20];
} File_System;

typedef struct file_descriptor {
    FD_INDEX index;
    File_System* fs;
    Disk* disk;

    // Used by the underline filesystem
    void* private_data;
} File_Descriptor;

void fs_init();
FD_INDEX fs_open(char* filename, FILE_MODE mode);
int fs_stat(FD_INDEX fd, File_Stat* stat);
int fs_close(FD_INDEX fd);
int fs_seek(FD_INDEX fd, uint32_t offset, SEEK_MODE mode);
int fs_read (void* ptr, FD_INDEX fd, uint32_t size, uint32_t nmemb);
void fs_insert_filesystem(File_System* fs);
File_System* fs_resolve(Disk* disk);

#endif
