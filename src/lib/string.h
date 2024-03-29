#ifndef STRING_H
#define STRING_H

#include <stddef.h>
#include <stdint.h>

#define EOL '\n'
#define CHAR_0 48
#define CHAR_MINUS 45
#define STR_NULL_TERMINATE '\0'

char* itoa(int number);
size_t strlen(const char* str);
int atoi(const char* number);
uint8_t is_digit(char c);
int8_t get_digit(char c);
int8_t str_cmp(const char* str1, const char* str2);
int8_t str_icmp(const char* str1, const char* str2);
void str_ref_copy(char* to, const char* from);
char* str_copy(const char* str);
void str_slice(char* output, const char* str, size_t start, size_t end);
int str_index_of(const char* str, char needle);
char str_to_lower(char c);

#endif
