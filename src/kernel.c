#include "kernel.h"
#include "./lib/terminal.h"

void kernel_main() {
    terminal_init();
    println("Hello world!");
    println("Nice to see you again");
    println("Nice to see you again");
}
