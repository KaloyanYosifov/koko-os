#include "fat16.h"

#include "../../../kernel.h"
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
    FAT_ITEM_TYPE_FILE = 1
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
    char filename[8];
    char ext[3];
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

typedef struct fat_file_descriptor {
    Fat_Item* item;
    uint32_t pos;
} Fat_File_Descriptor;

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

int fat16_resolve(Disk* disk);
int fat16_stat(Disk* disk, void* descriptor, File_Stat* stat);
void* fat16_open(Disk* disk, Path_Part* path, FILE_MODE mode);
int fat16_seek(void* descriptor, uint32_t offset, SEEK_MODE mode);
int fat16_read(Disk* disk, char* out, void* descriptor, uint32_t size, uint32_t nmemb);
int fat16_close(void* descriptor);

File_System fat16_file_system = {
    .open = fat16_open,
    .seek = fat16_seek,
    .read = fat16_read,
    .stat = fat16_stat,
    .close = fat16_close,
    .resolve = fat16_resolve
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

    if (disk_stream_read(stream, dir, root_directory_size) != OK) {
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

static void fat16_to_proper_string(char** out, const char* in)
{
    while(*in != 0x00 && *in != 0x20)
    {
        **out = *in;
        *out += 1;
        in +=1;
    }

    if (*in == 0x20)
    {
        **out = 0x00;
    }
}

static void fat16_get_full_relative_filename(struct fat_directory_item* item, char* out, int max_len)
{
    memset(out, 0x00, max_len);
    char *out_tmp = out;
    fat16_to_proper_string(&out_tmp, (const char*) item->filename);
    if (item->ext[0] != 0x00 && item->ext[0] != 0x20)
    {
        *out_tmp++ = '.';
        fat16_to_proper_string(&out_tmp, (const char*) item->ext);
    }

}

static unsigned int fat16_get_first_cluster(Fat_Directory_Item* item) {
    return item->high_16_bits_first_cluster | item->low_16_bits_first_cluster;
}

// Ref: https://people.cs.umass.edu/~liberato/courses/2017-spring-compsci365/lecture-notes/11-fats-and-directory-entries/
static unsigned int fat16_cluster_to_sector(Fat_Private* private, unsigned int cluster) {
    unsigned int relative_sector_location = (cluster - 2) * private->header.primary_header.sectors_per_cluster;
    return private->root_directory.ending_sector_pos + relative_sector_location;
}

static unsigned int fat16_get_cluster_size(Disk* disk) {
    Fat_Private* private = disk->fs_private;

    return fat16_sector_to_absolute_pos(disk, private->header.primary_header.sectors_per_cluster);
}

static unsigned int fat16_get_first_fat_sector(Fat_Private* private) {
    return private->header.primary_header.reserved_sectors;
}

static int fat16_get_fat_entry(Disk* disk, unsigned int cluster) {
    Fat_Private* private = disk->fs_private;
    Disk_Stream* stream = private->fat_read_stream;

    if (!stream) {
        // TODO: change error
        return SYSTEM_FAIL;
    }

    unsigned int fat_table_position = fat16_get_first_fat_sector(private);

    if (disk_stream_seek(stream, fat_table_position * (cluster * KERNEL_FAT16_FAT_ENTRY_SIZE)) != OK) {
        // TODO: change error
        return SYSTEM_FAIL;
    }

    uint16_t result = 0;

    if(disk_stream_read(stream, &result, sizeof(result)) != OK) {
        // TODO: change error
        return SYSTEM_FAIL;
    }

    return result;
}

static bool fat16_is_entry_invalid(int entry) {
    return entry == 0xFF8 ||
        entry == 0xFFF ||
        entry == 0xFF0 ||
        entry == 0xFF6 ||
        entry == 0x00 ||
        entry == KERNEL_FAT16_BAD_SECTOR;
}

static bool fat16_is_entry_free(int entry) {
    return entry == 0x00;
}

static int fat16_get_cluster_for_offset(Disk* disk, unsigned int cluster, int offset) {
    unsigned int size_of_cluster_bytes = fat16_get_cluster_size(disk);
    unsigned cluster_ahead = offset / size_of_cluster_bytes;

    for (int i = 0; i< cluster_ahead; i++) {
        int entry = fat16_get_fat_entry(disk, cluster);

        if (fat16_is_entry_invalid(entry)) {
            return SYSTEM_FAIL;
        }

        cluster = entry;
    }

    return cluster;
}

static int fat16_read_internal_from_stream(Disk* disk, Disk_Stream* stream, unsigned int cluster, int offset, unsigned int total, void* out) {
    Fat_Private* private = disk->fs_private;
    unsigned int size_of_cluster_bytes = fat16_get_cluster_size(disk);

    cluster = fat16_get_cluster_for_offset(disk, cluster, offset);

    // error
    if (cluster < 0) {
        return INVALID_ARGUMENT;
    }

    unsigned int offset_from_cluster = offset % size_of_cluster_bytes;
    unsigned int start_sector = fat16_cluster_to_sector(private, cluster);
    unsigned int start_pos = fat16_sector_to_absolute_pos(disk, start_sector) + offset_from_cluster;
    unsigned int total_to_read = total > size_of_cluster_bytes ? size_of_cluster_bytes : total;

    if (disk_stream_seek(stream, start_pos) != OK) {
        return SYSTEM_FAIL;
    }

    if (disk_stream_read(stream, out, total_to_read) != OK) {
        return SYSTEM_FAIL;
    }

    total -= total_to_read;

    // continue reading the leftovers
    if (total > 0) {
        return fat16_read_internal_from_stream(disk, stream, cluster, offset + total_to_read, total, out + total_to_read);
    }

    return OK;
}

void fat16_free_directory(Fat_Directory* directory) {
    if (!directory) {
        return;
    }

    if (directory->item) {
        free(directory->item);
    }

    free(directory);
}

void fat16_free_item(Fat_Item* item) {
    if (item->type == FAT_ITEM_TYPE_DIRECTORY) {
        fat16_free_directory(item->directory);
        free(item);
    } else if (item->type == FAT_ITEM_TYPE_FILE) {
        free(item->item);
        free(item);
    } else {
        panic("Invalid fat16 file");
    }
}

static int fat16_read_internal(Disk* disk, unsigned int cluster, int offset, unsigned int total, void* out) {
    // use cluster read stream from private
    Fat_Private* private = disk->fs_private;
    Disk_Stream* stream = private->cluster_read_stream;

    // and use read internal from stream function
    return fat16_read_internal_from_stream(disk, stream, cluster, offset, total, out);
}

static Fat_Directory_Item* fat16_clone_directory_item(Fat_Directory_Item* dir_item, unsigned int size) {
    if (size < sizeof(struct fat_directory_item))
    {
        return 0;
    }

    Fat_Directory_Item* copy_dir_item =zalloc(size);

    if (!copy_dir_item)
    {
        return NULL;
    }

    memcpy(copy_dir_item, dir_item, size);

    return copy_dir_item;
}

static Fat_Directory* fat16_load_fat_directory(Disk* disk, Fat_Directory_Item* dir_item) {
    // check if item attribute for a directory
    // if not return null
    if (!(dir_item->attributes & FAT_FILE_ATTRIBUTES_SUBDIRECTORY)) {
        return NULL;
    }

    // initialize directory instance
    Fat_Directory* directory = zalloc(sizeof(Fat_Directory));

    if (!directory) {
        return NULL;
    }

    Fat_Private* private = disk->fs_private;
    unsigned int cluster = fat16_get_first_cluster(dir_item);
    unsigned int cluster_sector = fat16_cluster_to_sector(private, cluster);
    unsigned int total_items = fat16_get_total_items_for_directory(disk, cluster_sector);
    unsigned int directory_size = directory->total * sizeof(Fat_Directory_Item);

    directory->total = total_items;
    directory->item = zalloc(directory_size);

    if (!directory->item) {
        free(directory);

        return NULL;
    }

    if (fat16_read_internal(disk, cluster, 0x00, directory_size, directory->item) != OK) {
        free(directory);
        free(directory->item);

        return NULL;
    }

    return directory;
}

static Fat_Item* fat16_new_fat_item_for_directory_item(Disk* disk, Fat_Directory_Item* dir_item) {
    // create a fat item and zalloc it
    Fat_Item* item = zalloc(sizeof(Fat_Item));

    // check if we have memory
    if (!item) {
        return NULL;
    }

    // check if the item attribute is a subdirectory
    // and set it's property
    if (dir_item->attributes & FAT_FILE_ATTRIBUTES_SUBDIRECTORY) {
        item->directory = fat16_load_fat_directory(disk, dir_item);
        item->type = FAT_ITEM_TYPE_DIRECTORY;
    } else {
        // else set the file attributes
        item->type = FAT_ITEM_TYPE_FILE;
    }

    // clone directory item
    item->item = fat16_clone_directory_item(dir_item, sizeof(Fat_Directory_Item));

    return item;
}

static Fat_Item* fat16_find_item_in_directory(Disk* disk, Fat_Directory* directory, const char* name) {
    Fat_Item* item = NULL;
    char tmp_filename[KERNEL_MAX_PATH_SIZE];

    // loop through directory items and get their full name
    for (unsigned int i = 0; i < directory->total; i++) {
        Fat_Directory_Item* dir_item = &directory->item[i];

        // and check if that name equals the name that we have as an argument
        fat16_get_full_relative_filename(dir_item, tmp_filename, sizeof(tmp_filename));

        if (str_icmp(tmp_filename, name) == OK)
        {
            item = fat16_new_fat_item_for_directory_item(disk, dir_item);
        }
    }

    return item;
}

static Fat_Item* fat16_get_directory_entry(Disk* disk, Path_Part* path) {
    Fat_Private* fat_private = disk->fs_private;
    Fat_Item* current_item = NULL;
    Fat_Item* root_item = fat16_find_item_in_directory(disk, &fat_private->root_directory, path->name);

    if (!root_item) {
        return NULL;
    }

    Path_Part* next_part = path->next;
    current_item = root_item;

    while (next_part != NULL) {
        if (current_item->type != FAT_ITEM_TYPE_DIRECTORY) {
            current_item = NULL;
            break;
        }

        Fat_Item* tmp_item = fat16_find_item_in_directory(disk, current_item->directory, next_part->name);
        // we free as we are cloning the item instead of getting the reference
        fat16_free_item(current_item);
        current_item = tmp_item;
        next_part = next_part->next;
    }

    return current_item;
}

static void* fat16_open_for_read(Disk* disk, Path_Part* path, FILE_MODE mode) {
    Fat_File_Descriptor* descriptor = zalloc(sizeof(Fat_File_Descriptor));

    if (!descriptor) {
        return ERROR(HEAP_MEMORY_NOT_MEMORY_LEFT);
    }

    descriptor->item = fat16_get_directory_entry(disk, path);

    if (!descriptor->item) {
        free(descriptor);

        return ERROR(FS_CANNOT_GET_FILE);
    }

    descriptor->pos = 0;

    return descriptor;
}

static Fat_Item* fat16_new_file_item() {
    Fat_Item* item = zalloc(sizeof(Fat_Directory_Item));
    Fat_Directory_Item* dir_item = zalloc(sizeof(Fat_Directory_Item));

    item->item = dir_item;
    item->type = FAT_ITEM_TYPE_FILE;

    // take name as argument instead of hardcoding
    str_ref_copy(dir_item->filename, "koko    ");
    str_ref_copy(dir_item->ext, "txt");

    return item;
}

static void* fat16_open_for_write(Disk* disk, Path_Part* path, FILE_MODE mode) {
    Fat_File_Descriptor* descriptor = zalloc(sizeof(Fat_File_Descriptor));

    if (!descriptor) {
        return ERROR(HEAP_MEMORY_NOT_MEMORY_LEFT);
    }

    descriptor->item = fat16_get_directory_entry(disk, path);

    if (!descriptor->item) {
        descriptor->item = fat16_new_file_item();
    }

    descriptor->pos = 0;

    return descriptor;
}

void* fat16_open(Disk* disk, Path_Part* path, FILE_MODE mode) {
    switch(mode) {
        case FILE_MODE_READ:
            return fat16_open_for_read(disk, path, mode);
        case FILE_MODE_WRITE:
            return fat16_open_for_write(disk, path, mode);
        default:
            return ERROR(INVALID_ARGUMENT);
    }
}

File_System* fat16_init() {
    str_ref_copy(fat16_file_system.name, "FAT 16");

    return &fat16_file_system;
}

static unsigned int fat16_get_cluster_root_offset() {
    return 2;
}

static unsigned int fat16_end_cluster(Disk* disk) {
    Fat_Private* private = disk->fs_private;
    unsigned int total_fat_size = fat16_sector_to_absolute_pos(disk, private->header.primary_header.sectors_big);

    return (total_fat_size / fat16_get_cluster_size(disk)) + fat16_get_cluster_root_offset();
}

static int fat16_get_free_cluster(Disk* disk, unsigned int total) {
    unsigned int clusters_required = total / fat16_get_cluster_size(disk);
    unsigned int start_cluster = 1 + fat16_get_cluster_root_offset();
    unsigned int end_cluster = start_cluster;
    unsigned int end_cluster_of_fat = fat16_end_cluster(disk);

    while (true) {
        int entry = fat16_get_fat_entry(disk, start_cluster);

        if (!fat16_is_entry_free(entry)) {
            start_cluster = end_cluster + 1;
            end_cluster = start_cluster;
            continue;
        }

        if (end_cluster > end_cluster_of_fat) {
            // TODO: use a better error code
            return SYSTEM_FAIL;
        }

        bool desired_cluster_allocations = (start_cluster + clusters_required) >= end_cluster;

        if (desired_cluster_allocations) {
            break;
        }

        end_cluster++;
    }

    return start_cluster;
}

int fat16_read(Disk* disk, char* out, void* descriptor, uint32_t size, uint32_t nmemb) {
    Fat_File_Descriptor* desc = descriptor;

    if (desc->item->type != FAT_ITEM_TYPE_FILE) {
        return INVALID_ARGUMENT;
    }

    Fat_Directory_Item* item = desc->item->item;

    unsigned int offset = desc->pos;
    char* temp_out = out;

    for (uint32_t i = 0; i < nmemb; i++) {
        if(fat16_read_internal(disk, fat16_get_first_cluster(item), offset, size, temp_out) != OK) {
            return DISK_FAIL_TO_READ_STREAM;
        }

        temp_out += size;
        offset += size;
    }

    return OK;
}

int fat16_seek(void* descriptor, uint32_t offset, SEEK_MODE mode) {
    Fat_File_Descriptor* desc = descriptor;

    if (desc->item->type != FAT_ITEM_TYPE_FILE) {
        return INVALID_ARGUMENT;
    }

    Fat_Directory_Item* item = desc->item->item;

    if (offset >= item->filesize) {
        // TODO: use a better error
        return SYSTEM_FAIL;
    }

    switch(mode) {
        case SEEK_MODE_SET:
            desc->pos = offset;

            break;
        case SEEK_MODE_END:
            return NOT_IMPLEMENTED;
        case SEEK_MODE_CUR:
            if (desc->pos + offset >= item->filesize) {
                // TODO: use a better error
                return SYSTEM_FAIL;
            }

            desc->pos += offset;

            break;
        default:
            return INVALID_ARGUMENT;
    }

    return OK;
}

int fat16_stat(Disk* disk, void* descriptor, File_Stat* stat) {
    Fat_File_Descriptor* desc = descriptor;

    if (desc->item->type != FAT_ITEM_TYPE_FILE) {
        return INVALID_ARGUMENT;
    }

    Fat_Directory_Item* item = desc->item->item;

    stat->flags = 0x00;
    stat->size = item->filesize;

    if (item->attributes & FAT_FILE_ATTRIBUTES_READ_ONLY) {
        stat->flags = FILE_STAT_READ_ONLY;
    }

    return OK;
}

int fat16_close(void* descriptor) {
    Fat_File_Descriptor* desc = descriptor;

    fat16_free_item(desc->item);
    free(desc);

    return OK;
}
