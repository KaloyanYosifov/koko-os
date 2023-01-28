#include "kernel.h"
#include "config.h"
#include "stdbool.h"
#include "idt/idt.h"
#include "lib/string.h"
#include "lib/io/disk.h"
#include "lib/terminal.h"
#include "lib/memory/memory.h"
#include "lib/memory/paging.h"
#include "lib/fs/path_parser.h"
#include "lib/fs/disk_stream.h"

extern void kernel_enable_interrupts();
extern void kernel_disable_interrupts();

Paging_Chunk* kernel_chunk;

void panic(const char* message) {
    println(message);

    while (true) {}
}

void init_kernel_paging() {
    kernel_chunk = paging_create_chunk(PAGING_PAGE_IS_WRITABLE | PAGING_PAGE_IS_PRESENT | PAGING_PAGE_ALLOW_ACCESS_TO_ALL);
    paging_switch_directory(kernel_chunk->directory);
}

void kernel_main() {
    kernel_disable_interrupts();

    terminal_init();
    memory_init();
    disk_init();
    idt_init();
    init_kernel_paging();
    paging_enable_paging();
    kernel_enable_interrupts();

    Disk_Stream* stream = disk_stream_new(0);
    disk_stream_seek(stream, 0x520);
    Disk_Stream_Read_Info info = disk_stream_read(stream, 536);

    println(info.buffer);
}
