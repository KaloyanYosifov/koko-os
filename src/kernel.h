#ifndef KERNEL_H
#define KERNEL_H

void kernel_main();
void panic(const char* message);
void kernel_enable_interrupts();
void kernel_disable_interrupts();

#endif
