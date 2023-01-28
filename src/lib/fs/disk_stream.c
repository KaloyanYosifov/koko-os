#include "disk_stream.h"

#include "../../config.h"
#include "../../errors.h"
#include "../memory/memory.h"

Disk_Stream* disk_stream_new(DISK_TYPE id) {
    Disk* disk = disk_get(id);

    if (!disk) {
        return NULL;
    }

    Disk_Stream* stream = malloc(sizeof(Disk_Stream));
    stream->pos = 0;
    stream->disk = disk;

    return stream;
}

 // create seek function

// create disk read function

void disk_stream_seek(Disk_Stream* stream, unsigned int pos) {
    stream->pos = pos;
}

Disk_Stream_Read_Info disk_stream_read(Disk_Stream* stream, unsigned int total_bytes) {
    Disk_Stream_Read_Info info;

    memset(&info, 0, sizeof(Disk_Stream_Read_Info));

    if (total_bytes == 0) {
        info.error_code = INVALID_ARGUMENT;

        return info;
    }

    unsigned int buffer_index = 0;
    char* buffer = malloc(sizeof(char) * total_bytes + 1);

    do {
        unsigned int current_total_read = total_bytes > KERNEL_DEFAULT_DISK_SECTOR_SIZE ? KERNEL_DEFAULT_DISK_SECTOR_SIZE : total_bytes;
        unsigned int sector = stream->pos / KERNEL_DEFAULT_DISK_SECTOR_SIZE;
        unsigned int offset = stream->pos % KERNEL_DEFAULT_DISK_SECTOR_SIZE;
        Disk_Sector_Info sector_info = disk_read_block(stream->disk, sector, 1);

        if (sector_info.error_code != OK) {
            info.error_code = INVALID_ARGUMENT;

            return info;
        }

        for (unsigned int i = 0; i < current_total_read; i++) {
            if (offset + i >= KERNEL_DEFAULT_DISK_SECTOR_SIZE) {
                // add to the total_bytes left for the next sector
                // if we have reached the end of the buffer
                current_total_read = current_total_read - i;
                total_bytes += current_total_read;

                break;
            }

            buffer[buffer_index++] = ((char*)sector_info.buffer)[offset+i];
        }

        free(sector_info.buffer);

        // if the total is less than a sector size
        // set it to 0 indicating that we have finished
        if (total_bytes <= KERNEL_DEFAULT_DISK_SECTOR_SIZE) {
            total_bytes = 0;
        } else {
            total_bytes -= KERNEL_DEFAULT_DISK_SECTOR_SIZE;
        }

        // move pos to the start position of next sector
        // we do this because, if we read a byte in sector 0 with offset 288 and we want to read 512 bytes
        // then we need to take 224 bytes from the next sector
        // therefor pos must start from the start of the next sector
        stream->pos += KERNEL_DEFAULT_DISK_SECTOR_SIZE - (stream->pos % KERNEL_DEFAULT_DISK_SECTOR_SIZE);
    } while (total_bytes > 0);

    buffer[buffer_index] = '\0';
    info.buffer = buffer;

    return info;
}
