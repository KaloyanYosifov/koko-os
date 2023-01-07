#include "kernel.h"
#include "stdbool.h"
#include "idt/idt.h"
#include "lib/string.h"
#include "lib/terminal.h"
#include "lib/memory/memory.h"

extern void kernel_enable_interrupts();

void panic(const char* message) {
    println(message);

    while (true) {}
}

void kernel_main() {
    terminal_init();
    memory_init();
    idt_init();

    println("Hello world!");
    println(itoa(-5000000));
    println(itoa(-500000000));
    println(itoa(500));
    println(itoa(44320));
    println(itoa(10));
    println(itoa(0));
}
