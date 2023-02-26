#include "string.h"

#include "memory/memory.h"

#include "../kernel.h"

size_t strlen(const char* str) {
    uint16_t len = 0;

    while (str[len] != STR_NULL_TERMINATE) {
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

    string[length - 1] = STR_NULL_TERMINATE;

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

    for (unsigned int i = 0; i < str1_len; i++) {
        if (str1[i] > str2[i]) {
            return 1;
        } else if (str1[i] < str2[i]) {
            return -1;
        }
    }

    return 0;
}

void str_ref_copy(char* to, const char* from) {
    char* result = to;

    while (*from != 0) {
        *result = *from;
        from++;
        result++;
    }

    *result = STR_NULL_TERMINATE;
}

char* str_copy(const char* str) {
    size_t str_size = strlen(str);

    if (str_size <= 0) {
        return "";
    }

    char* copy_str = malloc(sizeof(char) * (str_size + 1));
    unsigned int index = 0;

    while (str[index] != STR_NULL_TERMINATE) {
        copy_str[index] = str[index];
        index++;
    }

    copy_str[index] = STR_NULL_TERMINATE;

    return copy_str;
}

char* str_slice(const char* str, size_t start, size_t end) {
    if (start > end || start < 0 || end < 0) {
        return str_copy(str);
    }

    char* slice_str = malloc(sizeof(char) * (start + end) + 1);
    unsigned int index = 0;

    for (;start < end; start++) {
        slice_str[index++] = str[start];
    }

    slice_str[index] = STR_NULL_TERMINATE;

    return slice_str;
}

int str_index_of(const char* str, char needle) {
    if (str[0] == needle) {
        return 0;
    }

    size_t size_of_str = strlen(str);

    for (unsigned int i = 0; i < size_of_str; i++) {
        if (str[i] == needle) {
            return i;
        }
    }

    return -1;
}
