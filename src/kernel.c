#include "kernel.h"
#include "idt/idt.h"
#include "lib/terminal.h"
#include "lib/memory/memory.h"

extern void kernel_enable_interrupts();

typedef struct testing {
    uint32_t val;
    uint8_t val2;
    uint32_t val3;
} testing;

void kernel_main() {
    terminal_init();
    memory_init();
    idt_init();

    println("Hello world!");
    uint32_t* data = malloc(sizeof(uint32_t));
    void* something = malloc(3232);
    void* something2 = malloc(5433);
    uint32_t* data2 = malloc(sizeof(uint32_t));
    uint32_t* data3 = malloc(sizeof(uint32_t));

    *data = 11;
    *data2 = 22;
    *data3 = 33;

    print_number(*data);
    print_number(*data2);
    print_number(*data3);

    free(something);

    print_number((uint32_t) something2);

    uint32_t* data4 = malloc(sizeof(uint32_t));

    *data4 = 44;

    print_number(*data4);
}
