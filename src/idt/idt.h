#ifndef IDT_H
#define IDT_H

#include <stdint.h>

struct idt_desc
{
    // Offset bits 0-15
    uint16_t offset_1;
    // Selector from our GDT
    uint16_t selector;
    // Set to zero we do not need it
    uint8_t zero;
    // type and attributes
    uint8_t type_attr;
    // offset bits 16-32
    uint16_t offset_2;
} __attribute__((packed));

struct idtr_desc {
    // size of descriptor table - 1
    uint16_t limit;
    // Base address of the idt_desc
    uint32_t base;
} __attribute__((packed));


void idt_init();

#endif
