#include "kernel.h"
#include "config.h"
#include "stdbool.h"
#include "idt/idt.h"
#include "lib/string.h"
#include "lib/io/disk.h"
#include "lib/terminal.h"
#include "lib/memory/memory.h"
#include "lib/memory/paging.h"
#include "lib/fs/file.h"
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
    fs_init();
    disk_init();
    idt_init();
    init_kernel_paging();
    paging_enable_paging();
    kernel_enable_interrupts();

    char test[50];
    str_ref_copy(test, "Hello there world!");

    println(test);

    FD_INDEX fd = fs_open("0:/main.txt", FIEL_MODE_READ);

    if (fd) {
        println("File loaded!");
    }

    print_number(fd);
}
