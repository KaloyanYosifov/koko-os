#ifndef TERMINAL_H
#define TERMINAL_H

#include <stdint.h>

#define VGA_WIDTH 80
#define VGA_HEIGHT 20
#define VIDEO_MEMORY_LOCATION 0xB8000

uint16_t terminal_make_char(char character, uint8_t color_code);
void terminal_write_char(char character, uint8_t color_code);
void terminal_clear();
void terminal_newline();
void terminal_init();
void print_char(char character);
void print(char* msg);
void println(char* msg);
void print_number(uint32_t number);

#endif
