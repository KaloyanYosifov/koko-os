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

    unsigned int length = 0;
    unsigned int number_copy = number;;

    // reverse number
    while (number_copy > 0) {
        number_copy = number_copy / 10;

        length++;
    }

    unsigned int string_length = length + 1 + is_negative;
    // we add +1 for the last null character \0
    // + is_negative for the first negative character
    char* string = malloc(sizeof(char) * string_length);

    if (is_negative > 0) {
        string[0] = CHAR_MINUS;
    }

    // -2 as -1 will be for null character
    unsigned int index = string_length - 2;

    while (number > 0) {
        uint8_t n = number % 10;
        number = number / 10;

        string[index--] = (char) (CHAR_0 + n);
    }

    string[string_length - 1] = '\0';

    return string;
}
