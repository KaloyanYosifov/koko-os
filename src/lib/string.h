#ifndef STRING_H
#define STRING_H

#include <stddef.h>

#define EOL '\n'
#define CHAR_0 48
#define CHAR_MINUS 45

char* itoa(int number);
size_t strlen(const char* str);

#endif
