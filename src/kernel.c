#include "kernel.h"
#include "idt/idt.h"
#include "lib/terminal.h"

extern void kernel_enable_interrupts();

void kernel_main() {
    terminal_init();
    idt_init();

    print("Hello world!\n");
    /* println("Nice to see you again"); */
    /* println("Nice to see you again"); */
}
