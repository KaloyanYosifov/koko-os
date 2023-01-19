#include "path_parser.h"

#include "../string.h"
#include "../../config.h"
#include "../memory/memory.h"

bool path_parser_validate_path(const char* path) {
    if (strlen(path) > KERNEL_MAX_PATH_SIZE) {
        return false;
    }

    if (!is_digit(path[0])) {
        return false;
    }

    char* start_path_slice = str_slice(path, 1, 3);

    if (str_cmp(start_path_slice, ":/") != 0) {
        free(start_path_slice);

        return false;
    }

    free(start_path_slice);

    return true;
}

// TODO: create a function to parse whole path and return struct
