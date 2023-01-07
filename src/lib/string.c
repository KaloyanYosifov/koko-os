#include "string.h"
#include <stdint.h>

size_t strlen(const char* str) {
    uint16_t len = 0;

    while (str[len] != '\0') {
        len++;
    }

    return len;
}
