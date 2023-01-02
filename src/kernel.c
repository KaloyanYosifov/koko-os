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

    print("Hello world!\n");
    /* uint32_t* data = malloc(sizeof(uint32_t)); */
    /* uint32_t* data2 = malloc(sizeof(uint32_t)); */
    testing* test = malloc(sizeof(testing));
    testing* test2 = malloc(sizeof(testing));

    /* *data = 44; */
    /* *data2 = 33; */

    test->val = 90000;
    test->val2 = 243;
    test->val3 = 50000;

    test2->val = 33200;
    test2->val2 = 120;
    test2->val3 = 53342;

    /* print_number(*data + *data2); */
    /* println("Nice to see you again"); */
    /* println("Nice to see you again"); */
}
