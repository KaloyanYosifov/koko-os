#include "fat16.h"

#include "../disk_stream.h"
#include "../../string.h"
#include "../../../config.h"
#include "../../../errors.h"

#include <stddef.h>
#include <stdint.h>

#define KERNEL_FAT16_SIGNATURE 41
#define KERNEL_FAT16_FAT_ENTRY_SIZE 2
#define KERNEL_FAT16_BAD_SECTOR 0xFF7
#define KERNEL_FAT16_UNUSED 0

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
