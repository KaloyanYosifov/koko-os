#include "string.h"

#include "memory/memory.h"

#include "../kernel.h"

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

    size_t length = 0;
    unsigned int number_copy = number;

    // reverse number
    while (number_copy > 0) {
        number_copy = number_copy / 10;

        length++;
    }

    // resize length to accommodate for negative and \0 values
    length = length + 1 + is_negative;
    // we add +1 for the last null character \0
    // + is_negative for the first negative character
    char* string = malloc(sizeof(char) * length);

    if (is_negative != 0) {
        string[0] = CHAR_MINUS;
    }

    // -2 as -1 will be for null character
    unsigned int index = length - 2;

    while (number > 0) {
        uint8_t n = number % 10;
        number = number / 10;

        string[index--] = (char) (CHAR_0 + n);
    }

    string[length - 1] = '\0';

    return string;
}


uint8_t is_digit(char c) {
    return c >= 48 && c <= 57;
}

int8_t get_digit(char c) {
    return c - CHAR_0;
}

int atoi(const char* number) {
    size_t length = strlen(number);

    if (length == 0) {
        return 0;
    }

    uint8_t is_negative = 0;
    int assembled_number = 0;

    for (size_t i = 0; i < length; i++) {
        if (number[i] == CHAR_MINUS) {
            is_negative = 1;
            continue;
        }

        assembled_number *= 10;

        if (!is_digit(number[i])) {
            panic("String is not a number!");
        }

        assembled_number += get_digit(number[i]);
    }

    // if we need the number to be negative, convert it
    if (is_negative != 0) {
        assembled_number *= -1;
    }

    return assembled_number;
}

int8_t str_cmp(const char* str1, const char* str2) {
    size_t str1_len = strlen(str1);
    size_t str2_len = strlen(str2);

    if (str1_len > str2_len) {
        return 1;
    } else if (str1_len < str2_len) {
        return -1;
    }

    size_t i = 0;

    while (i < str1_len) {
        if (str1[i] > str2[i]) {
            return 1;
        } else if (str1[i] < str2[i]) {
            return -1;
        }
    }

    return 0;
}

char* str_slice(const char* str, size_t start, size_t end) {
    if (end > start) {
        return "";
    }

    char* slice_str = malloc(sizeof(char) * (start + end) + 1);

    for (;start < end; start++) {
        slice_str[start] = str[start];
    }

    slice_str[start] = '\0';

    return slice_str;
}
