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

    int res = OK;
    unsigned int leftover_sector = (total_bytes % KERNEL_DEFAULT_DISK_SECTOR_SIZE) && 1;
    unsigned int sectors_required_to_read = (total_bytes / KERNEL_DEFAULT_DISK_SECTOR_SIZE) + leftover_sector;
    unsigned int sector = stream->pos / KERNEL_DEFAULT_DISK_SECTOR_SIZE;
    unsigned int offset = stream->pos % KERNEL_DEFAULT_DISK_SECTOR_SIZE;
    char* local_buffer = malloc(sizeof(char) * (KERNEL_DEFAULT_DISK_SECTOR_SIZE * sectors_required_to_read));

    if (disk_read_block(stream->disk, local_buffer, sector, sectors_required_to_read) != OK) {
        // TODO: use better error code
        res = INVALID_ARGUMENT;

        goto fin;
    }

    char* temp_buffer = buffer;

    for (unsigned int i = 0; i < total_bytes; i++) {
        temp_buffer[i] = local_buffer[offset + i];
    }

    stream->pos += total_bytes;

fin:
    free(local_buffer);
    return res;
}

void disk_stream_close(Disk_Stream* stream) {
    free(stream);
}

void disk_stream_reset(Disk_Stream* stream) {
    stream->pos = 0;
}
