#include "idt.h"
#include "../kernel.h"
#include "../config.h"
#include "../lib/io/io.h"
#include "../lib/terminal.h"
#include "../lib/memory/memory.h"

struct idt_desc idt_descriptors[KERNEL_TOTAL_INTERRUPTS];
struct idtr_desc idtr_descriptor;

extern void idt_load(struct idtr_desc* ptr);
extern void idt_zero_interrupt();
extern void idt_null_interrupt();
extern void idt_keyboard_interrupt();

void acknowledge_interrupt() {
    outb(0x20, 0x20);
}

void idt_null() {
    acknowledge_interrupt();
}

void idt_zero() {
    println("Divide by zero error!");
}

void idt_keyboard() {
    println("A key");
    acknowledge_interrupt();
}

void idt_set(int interrupt_no, void* address) {
    struct idt_desc* desc = &idt_descriptors[interrupt_no];
    desc->offset_1 = (uint32_t) address & 0x0000ffff;
    desc->selector = KERNEL_CODE_SELETOR;
    desc->zero = 0;
    desc->type_attr = 0xee;
    desc->offset_2 = (uint32_t) address >> 16;
}

void idt_init() {
    memset(idt_descriptors, 0, sizeof(idt_descriptors));
    idtr_descriptor.limit = sizeof(idt_descriptors) - 1;
    idtr_descriptor.base = (uint32_t) idt_descriptors;

    for (uint16_t i = 0; i < KERNEL_TOTAL_INTERRUPTS; i++) {
        idt_set(i, &idt_null_interrupt);
    }

    idt_set(0, &idt_zero_interrupt);
    idt_set(KEYBOARD_INTERRUPT, &idt_keyboard);

    idt_load(&idtr_descriptor);

    kernel_enable_interrupts();
}
