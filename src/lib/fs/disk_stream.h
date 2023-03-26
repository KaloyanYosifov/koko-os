#ifndef DISK_STREAM_H
#define DISK_STREAM_H

#include "../io/disk.h"

typedef struct disk_stream {
    unsigned int pos;
    Disk* disk;
} Disk_Stream;

typedef struct disk_stream_read_info {
    int error_code;
    char* buffer;
} Disk_Stream_Read_Info;

Disk_Stream* disk_stream_new(DISK_TYPE id);
int disk_stream_seek(Disk_Stream* stream, unsigned int pos);
int disk_stream_read(Disk_Stream* stream, void* buffer, unsigned int total_bytes);
int disk_stream_write(Disk_Stream* stream, void* buffer, unsigned int total_bytes);
void disk_stream_close(Disk_Stream* stream);
void disk_stream_reset(Disk_Stream* stream);

#endif
