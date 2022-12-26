#include "terminal.h"
#include "string.h"

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
uint16_t terminal_make_char(char character, uint8_t color_code) {
    return (color_code << 8) | character;
}

void terminal_print_char(uint16_t x, uint16_t y, char character, uint8_t color_code) {
    video_mem[(y * VGA_WIDTH) + x] = terminal_make_char(character, color_code);
}

void terminal_write_char(char character, uint8_t color_code) {
    if (character == EOL) {
		current_terminal_x = 0;
		current_terminal_y += 1;

		return;
	} else if (current_terminal_x >= VGA_WIDTH) {
		current_terminal_x = 0;
		current_terminal_y += 1;
	}

	if (current_terminal_y >= VGA_HEIGHT) {
		current_terminal_y = 0;
	}

    video_mem[(current_terminal_y * VGA_WIDTH) + current_terminal_x] = terminal_make_char(character, color_code);
    current_terminal_x += 1;
}

void terminal_clear() {
    for (uint8_t x = 0; x < VGA_WIDTH; x++) {
        for (uint8_t y = 0; y < VGA_HEIGHT; y++) {
            terminal_print_char(x, y, '\0', 0);
        }
    }

    current_terminal_x = 0;
    current_terminal_y = 0;
}

void terminal_newline() {
    terminal_write_char(EOL, 0);
}

void terminal_init() {
    video_mem = (uint16_t*)(VIDEO_MEMORY_LOCATION);

    terminal_clear();
}

void print(char* msg) {
    size_t msg_len = strlen(msg);

    for (size_t i = 0; i < msg_len; i++) {
		terminal_write_char(msg[i], 0xF);
    }
}

void println(char* msg) {
    print(msg);

    terminal_newline();
}
