#include "kernel.h"
#include "stdbool.h"
#include "idt/idt.h"
#include "lib/string.h"
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

    char* ptr = zalloc(4096);
    paging_set(kernel_chunk->directory, (void*) 0x1000, ((uint32_t)ptr) | PAGING_PAGE_ALLOW_ACCESS_TO_ALL | PAGING_PAGE_IS_PRESENT | PAGING_PAGE_IS_WRITABLE);

    paging_enable_paging();

    char* ptr2 = (char*) 0x1000;
    ptr2[0] = 'H';
    ptr2[1] = 'E';
    println(ptr2);


    println(ptr);


    kernel_enable_interrupts();
}
