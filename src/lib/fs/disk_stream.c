#include "disk_stream.h"

#include "../terminal.h"
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

int disk_stream_seek(Disk_Stream* stream, unsigned int pos) {
    stream->pos = pos;

    return OK;
}

int disk_stream_read(Disk_Stream* stream, void* buffer, unsigned int total_bytes) {
    if (total_bytes == 0) {
        return INVALID_ARGUMENT;
    }

    char* copy_buffer = buffer;

    unsigned int buffer_index = 0;

    do {
        unsigned int current_total_read = total_bytes > KERNEL_DEFAULT_DISK_SECTOR_SIZE ? KERNEL_DEFAULT_DISK_SECTOR_SIZE : total_bytes;
        unsigned int sector = stream->pos / KERNEL_DEFAULT_DISK_SECTOR_SIZE;
        unsigned int offset = stream->pos % KERNEL_DEFAULT_DISK_SECTOR_SIZE;
        char local_buffer[KERNEL_DEFAULT_DISK_SECTOR_SIZE];

        if (disk_read_block(stream->disk, local_buffer, sector, 1) != OK) {
            // TODO: use better error code
            return INVALID_ARGUMENT;
        }

        for (unsigned int i = 0; i < current_total_read; i++) {
            if (offset + i >= KERNEL_DEFAULT_DISK_SECTOR_SIZE) {
                // add to the total_bytes left for the next sector
                // if we have reached the end of the buffer
                current_total_read = current_total_read - i;
                total_bytes += current_total_read;

                break;
            }

            copy_buffer[buffer_index++] = local_buffer[offset+i];
        }

        // if the total is less than a sector size
        // set it to 0 indicating that we have finished
        if (total_bytes <= KERNEL_DEFAULT_DISK_SECTOR_SIZE) {
            total_bytes = 0;
        } else {
            total_bytes -= KERNEL_DEFAULT_DISK_SECTOR_SIZE;
        }

        stream->pos += current_total_read;
    } while (total_bytes > 0);

    return OK;
}

void disk_stream_close(Disk_Stream* stream) {
    free(stream);
}

void disk_stream_reset(Disk_Stream* stream) {
    stream->pos = 0;
}
