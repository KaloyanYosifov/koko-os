#include "kernel.h"
#include "idt/idt.h"
#include "lib/terminal.h"
#include "lib/memory/memory.h"

extern void kernel_enable_interrupts();

void kernel_main() {
    terminal_init();
    memory_init();
    idt_init();

    print("Hello world!\n");
    /* println("Nice to see you again"); */
    /* println("Nice to see you again"); */
}
