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

    FD_INDEX fd = fs_open("0:/main.txt", FILE_MODE_READ);

    if (fd) {
        File_Stat stat;
        int code = fs_stat(fd, &stat);

        print_number(code);

        char* buffer = zalloc((sizeof(char) * stat.size) + 1);
        fs_read(buffer, fd, stat.size, 1);
        println(buffer);
        free(buffer);

        code = fs_stat(fd, &stat);
        print_number(code);
    }

    FD_INDEX fd2 = fs_open("0:/koko.txt", FILE_MODE_WRITE);

    if (fd2) {
        char* testing = "Welcome my lover, my brother";
        fs_write(testing, fd2, strlen(testing));

        fs_close(fd2);
    }

    FD_INDEX fd3 = fs_open("0:/koko.txt", FILE_MODE_READ);

    // TODO: fix why we cannot read file that we created from the kernel
    if (fd3) {
        char buffer[512];
        fs_read(buffer, fd, 512, 1);
        println(buffer);

        fs_close(fd3);
    }
}
