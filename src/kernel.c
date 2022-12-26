#include "kernel.h"
#include "idt/idt.h"
#include "lib/terminal.h"

extern void _problem();

void kernel_main() {
    terminal_init();
    idt_init();

    println("Hello world!");
    println("Nice to see you again");
    println("Nice to see you again");

	/* _problem(); */
}
