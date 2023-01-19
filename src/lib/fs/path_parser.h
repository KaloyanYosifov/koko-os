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

#endif
