#include "kernel.h"
#include "config.h"
#include "stdbool.h"
#include "idt/idt.h"
#include "lib/string.h"
#include "lib/io/disk.h"
#include "lib/terminal.h"
#include "lib/memory/memory.h"
#include "lib/memory/paging.h"

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
    idt_init();
    init_kernel_paging();
    paging_enable_paging();
    kernel_enable_interrupts();

    Disk_Sector_Info info = disk_read_sector(0, 1);

    println("test");
    println((char*) info.buffer);
}
