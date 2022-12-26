#include "kernel.h"
#include "./lib/terminal.h"

void kernel_main() {
    terminal_init();
    println("Hello world!", 0xF);
    println("Nice to see you again", 0xF);
    println("Nice to see you again", 0xF);
}
