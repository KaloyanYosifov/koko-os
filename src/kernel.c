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
    int n1 = atoi("1234");
    int n2 = atoi("-1234");
    int n3 = atoi("100");
    int n4 = atoi("540");

    println(itoa(n1 + n2));
    println(itoa(n3 + n4));
}
