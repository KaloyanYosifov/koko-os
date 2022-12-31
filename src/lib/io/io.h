#ifndef IO_H
#define IO_H

#include <stdint.h>

unsigned char insb(uint16_t port);
uint16_t insw(uint16_t port);

void outb(uint16_t port, unsigned char val);
void outw(uint16_t port, uint16_t val);


#endif
