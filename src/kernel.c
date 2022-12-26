#include "kernel.h"
#include <stdint.h>
#include <stddef.h>

uint16_t* video_mem = 0;
uint16_t current_terminal_x = 0;
uint16_t current_terminal_y = 0;

/**
    * Combine character and color code into one 16 byte value
    * Example:
    * character = 65 or 0x41
    * color_code = 3 or 0x03 (cyan)
    * Since we have a little endian system, the end result must be 0x0341
    * To do this we shift by a byte or (8 bits) to the left to get 0x0300
    * The we or with 0x41 (0x0300 | 0x41) which equals 0x0341
*/
uint16_t make_terminal_char(char character, uint8_t color_code) {
    return (color_code << 8) | character;
}

void print_terminal_char(uint16_t x, uint16_t y, char character, uint8_t color_code) {
    video_mem[(y * VGA_WIDTH) + x] = make_terminal_char(character, color_code);
}

void clear_terminal() {
    for (uint8_t x = 0; x < VGA_WIDTH; x++) {
        for (uint8_t y = 0; y < VGA_HEIGHT; y++) {
            print_terminal_char(x, y, '\0', 0);
        }
    }

    current_terminal_x = 0;
    current_terminal_y = 0;
}

void newline_on_terminal() {
    current_terminal_x = 0;
    current_terminal_y += 1;
}

void initialize_terminal() {
    video_mem = (uint16_t*)(VIDEO_MEMORY_LOCATION);

    clear_terminal();
}

size_t strlen(char* str) {
    uint16_t len = 0;

    while (str[len] != '\0') {
        len++;
    }

    return len;
}

void print(char* msg, uint8_t color_code) {
    size_t msg_len = strlen(msg);

    for (size_t i = 0; i < msg_len; i++) {
        if (msg[i] == EOL) {
            newline_on_terminal();

            continue;
        }

        print_terminal_char(current_terminal_x, current_terminal_y, msg[i], color_code);

        current_terminal_x++;
    }
}

void println(char* msg, uint8_t color_code) {
    print(msg, color_code);

    newline_on_terminal();
}

void kernel_main() {
    initialize_terminal();
    println("Hello world!", 0xF);
    println("Nice to see you again", 0xF);
    println("Nice to see you again", 0xF);
}
