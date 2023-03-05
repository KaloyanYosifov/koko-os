#include "fat16.h"

#include "../disk_stream.h"
#include "../../string.h"
#include "../../../config.h"
#include "../../../errors.h"
#include "../../memory/memory.h"

#include <stddef.h>
#include <stdint.h>

#define KERNEL_FAT16_SIGNATURE 41
#define KERNEL_FAT16_FAT_ENTRY_SIZE 2
#define KERNEL_FAT16_BAD_SECTOR 0xFF7
#define KERNEL_FAT16_UNUSED 0
#define KERNEL_FAT16_END_OF_BLOCK 0
#define KERNEL_FAT16_EMPTY_ITEM 0xe5

// fat item attributes
typedef enum fat_item_type {
    FAT_ITEM_TYPE_DIRECTORY = 0,
    FAT_IEM_TYPE_FILE = 1
} FAT_ITEM_TYPE;

typedef enum fat_file_attributes {
    FAT_FILE_ATTRIBUTES_READ_ONLY = 0x01,
    FAT_FILE_ATTRIBUTES_HIDDEN = 0x02,
    FAT_FILE_ATTRIBUTES_SYSTEM = 0x04,
    FAT_FILE_ATTRIBUTES_VOLUME_LABEL = 0x08,
    FAT_FILE_ATTRIBUTES_SUBDIRECTORY = 0x10,
    FAT_FILE_ATTRIBUTES_ARCHIVED = 0x20,
    FAT_FILE_ATTRIBUTES_DEVICE = 0x40,
    FAT_FILE_ATTRIBUTES_RESERVED = 0x80,
} FAT_FILE_ATTRIBUTES;

typedef struct fat_extended_header {
    uint8_t drive_number;
    uint8_t win_nt_bit;
    uint8_t boot_signature;
    uint32_t volume_id;
    uint8_t volume_label[11];
    uint8_t fat_type_label[8];
} __attribute__((packed)) Fat_Extended_Header;

typedef struct main_fat_header {
    uint8_t bootjmp[3];
    uint8_t oem_id[8];
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t fat_copies;
    uint16_t root_dir_entries;
    uint16_t number_of_sectors;
    uint8_t media_type;
    uint16_t sectors_per_fat;
    uint16_t sectors_per_track;
    uint16_t number_of_heads;
    uint32_t hidden_sectors;
    uint32_t sectors_big;
} __attribute__((packed)) Main_Fat_Header;

typedef struct fat_header {
    Main_Fat_Header primary_header;
    union U_Fat_Extended_Header {
        Fat_Extended_Header extended_header;
    } shared;
} Fat_Header;

typedef struct fat_directory_item {
    uint8_t filename[8];
    uint8_t ext[3];
    uint8_t attributes;
    uint8_t reserved;
    uint8_t creation_time_tenths_of_a_sec;
    uint16_t creation_time;
    uint16_t creation_date;
    uint16_t last_access;
    uint16_t high_16_bits_first_cluster;
    uint16_t last_mod_time;
    uint16_t last_mod_date;
    uint16_t low_16_bits_first_cluster;
    uint32_t filesize;
} __attribute__((packed)) Fat_Directory_Item;

typedef struct fat_directory {
    Fat_Directory_Item* item;
    int total;
    int sector_pos;
    int ending_sector_pos;
} Fat_Directory;

typedef struct fat_item {
    // we create a union here
    // to represent that we are either going to have an item or directory
    // as union shares memory for the properties declared inside
    // only one of them will be present
    union {
        Fat_Directory_Item* item;
        Fat_Directory* directory;
    };

    FAT_ITEM_TYPE type;
} Fat_Item;

typedef struct fat_item_descriptor {
    Fat_Item* item;
    uint32_t pos;
} Fat_item_descriptor;

typedef struct fst_private {
    Fat_Header header;
    Fat_Directory root_directory;

    // used to stream data clusters
    Disk_Stream* cluster_read_stream;

    // used to steam the file allocation table
    Disk_Stream* fat_read_stream;

    // Use for directory streams
    Disk_Stream* directory_stream;
} Fat_Private;

void* fat16_open(Disk* disk, Path_Part* path, FILE_MODE mode);
int fat16_resolve(Disk* disk);

File_System fat16_file_system = {
    .resolve = fat16_resolve,
    .open = fat16_open
};

Fat_Private* fat16_init_private_data(Disk* disk) {
    Fat_Private* private = zalloc(sizeof(Fat_Private));

    private->fat_read_stream = disk_stream_new(disk->id);
    private->directory_stream = disk_stream_new(disk->id);
    private->cluster_read_stream = disk_stream_new(disk->id);

    return private;
}

void fat16_reset_private_disk_streams(Fat_Private* private_data) {
    disk_stream_reset(private_data->fat_read_stream);
    disk_stream_reset(private_data->directory_stream);
    disk_stream_reset(private_data->cluster_read_stream);
}

unsigned int fat16_sector_to_absolute_pos(Disk* disk, unsigned int sector)
{
    return sector * disk->sector_size;
}

int fat16_get_total_items_for_directory(Disk* disk, unsigned int start_sector) {
    Fat_Directory_Item item;

    memset(&item, 0, sizeof(Fat_Directory_Item));

    unsigned int directory_start_pos = fat16_sector_to_absolute_pos(disk, start_sector);
    Disk_Stream* stream = ((Fat_Private*)disk->fs_private)->directory_stream;

    if (disk_stream_seek(stream, directory_start_pos) != OK) {
        return DISK_FAIL_SET_STREAM_POS;
    }

    int items_count = 0;
    while(true) {
        if (disk_stream_read(stream, &item, sizeof(Fat_Directory_Item)) != OK) {
            return DISK_FAIL_TO_READ_STREAM;
        }

        // we reached the end
        if (item.filename[0] == KERNEL_FAT16_END_OF_BLOCK) {
            break;
        }

        // empty item found
        if (item.filename[0] == KERNEL_FAT16_EMPTY_ITEM) {
            continue;
        }

        items_count++;
    }

    return items_count;
}

int fat16_get_root_directory(Disk* disk, Fat_Private* private_data) {
    Main_Fat_Header* primary_header = &private_data->header.primary_header;
    int root_directory_sector = (primary_header->fat_copies * primary_header->sectors_per_fat) + primary_header->reserved_sectors;
    int root_directory_entries = primary_header->root_dir_entries;
    int root_directory_size = (root_directory_entries * sizeof(Fat_Directory_Item));
    int total_sectors = root_directory_size / disk->sector_size;

    if (root_directory_size % disk->sector_size != 0) {
        total_sectors += 1;
    }

    int total_items = fat16_get_total_items_for_directory(disk, root_directory_sector);

    if (total_items < 0) {
        return INVALID_ARGUMENT;
    }

    Fat_Directory_Item* dir = zalloc(root_directory_size);

    if (!dir) {
        return HEAP_MEMORY_NOT_MEMORY_LEFT;
    }

    Disk_Stream* stream = private_data->directory_stream;

    if (disk_stream_seek(stream, fat16_sector_to_absolute_pos(disk, root_directory_sector)) != OK) {
        return DISK_FAIL_SET_STREAM_POS;
    }

    if (disk_stream_read(stream, dir, sizeof(Fat_Directory_Item)) != OK) {
        return DISK_FAIL_TO_READ_STREAM;
    }

    Fat_Directory* primary_directory = &private_data->root_directory;

    primary_directory->item = dir;
    primary_directory->total = total_items;
    primary_directory->sector_pos = root_directory_sector;
    primary_directory->ending_sector_pos = root_directory_sector + (root_directory_size / disk->sector_size);

    return OK;
}

int fat16_resolve(Disk* disk) {
    int result = 0;
    Fat_Private* fat_private = fat16_init_private_data(disk);

    disk->fs_private = fat_private;
    disk->fs = &fat16_file_system;

    Disk_Stream* stream = disk_stream_new(disk->id);

    if (!stream) {
        result = SYSTEM_FAIL;
        goto fin;
    }

    if (disk_stream_read(stream, &fat_private->header, sizeof(Fat_Header)) != OK) {
        result = SYSTEM_FAIL;
        goto fin;
    }

    if (fat_private->header.shared.extended_header.boot_signature != KERNEL_FAT16_SIGNATURE) {
        result = WRONG_FILESYSTEM;
        goto fin;
    }

    if (fat16_get_root_directory(disk, fat_private) != OK)
    {
        result = SYSTEM_FAIL;

        goto fin;
    }

fin:
    if (stream) {
        disk_stream_close(stream);
    }

    if (result < 0) {
        free(fat_private);

        disk->fs = NULL;
        disk->fs_private = NULL;
    }

    fat16_reset_private_disk_streams(fat_private);

    return result;
}

void* fat16_open(Disk* disk, Path_Part* path, FILE_MODE mode) {
    return NULL;
}

File_System* fat16_init() {
    str_ref_copy(fat16_file_system.name, "FAT 16");

    return &fat16_file_system;
}
