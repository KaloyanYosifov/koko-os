#include "string.h"
#include "memory/memory.h"

#include <stdint.h>

size_t strlen(const char* str) {
    uint16_t len = 0;

    while (str[len] != '\0') {
        len++;
    }

    return len;
}

char* itoa(int number) {
    if (number == 0) {
        return "0";
    }

    uint8_t is_negative = 0;

    if (number < 0) {
        is_negative = 1;

        // make number positive
        number *= -1;
    }

    uint8_t zero_char = 48;
    uint8_t leading_zeroes = 0;

    unsigned int length = 0;
    unsigned int reversed = 0;

    // reverse number
    while (number > 0) {
        uint8_t n = number % 10;
        number = number / 10;

        length++;

        // if we have leading zeros append them to character
        if (n == 0 && reversed == 0) {
            leading_zeroes++;
            continue;
        }

        reversed *= 10;
        reversed += n;
    }

    // we add +1 for the last null character \0
    // + is_negative for the first negative character
    char* string = malloc(sizeof(char) * (length + 1 + is_negative));
    unsigned int index = 0;

    if (is_negative > 0) {
        string[index++] = 45;
    }

    while (reversed > 0) {
        uint8_t n = reversed % 10;
        reversed = reversed / 10;

        string[index++] = (char) (zero_char + n);
    }

    while (leading_zeroes > 0) {
        string[index++] = zero_char;

        leading_zeroes--;
    }

    string[index] = '\0';

    return string;
}
