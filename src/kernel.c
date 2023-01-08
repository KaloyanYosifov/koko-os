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
    paging_enable_paging();
}

void kernel_main() {
    kernel_disable_interrupts();

    terminal_init();
    memory_init();
    init_kernel_paging();
    idt_init();

    kernel_enable_interrupts();

    println("Hello world!");
    int n1 = atoi("1234");
    int n2 = atoi("-1234");
    int n3 = atoi("100");
    int n4 = atoi("540");

    println(itoa(n1 + n2));
    println(itoa(n3 + n4));
}
