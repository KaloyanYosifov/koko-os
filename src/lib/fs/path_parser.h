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

typedef struct path_parser_info {
    uint8_t err_code;
    Path_Root* root;
} Path_Parser_Info;

bool path_parser_is_path_valid(const char* path);
Path_Parser_Info path_parser_parse_path(const char* path);
void path_parser_free_root(Path_Root* root);

#endif
