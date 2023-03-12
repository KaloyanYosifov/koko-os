#ifndef KERNEL_H
#define KERNEL_H

#define ERROR(value) (void*)value

void kernel_main();
void panic(const char* message);
void kernel_enable_interrupts();
void kernel_disable_interrupts();

#endif
