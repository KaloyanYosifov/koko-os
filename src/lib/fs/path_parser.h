#ifndef PATH_PARSER_H
#define PATH_PARSER_H

#include <stdint.h>
#include <stdbool.h>

typedef struct path_part {
    char* name;
    struct path_part* next;
} Path_Part;

typedef struct path_root {
    uint8_t driver_no;
    Path_Part* part;
} Path_Root;

bool path_parser_is_path_valid(const char* path);
Path_Root* path_parser_parse_path(const char* path);
void path_parser_free_root(Path_Root* root);

#endif
