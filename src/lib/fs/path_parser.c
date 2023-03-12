#include "path_parser.h"

#include "../string.h"
#include "../../config.h"
#include "../../errors.h"
#include "../memory/memory.h"

bool path_parser_is_path_valid(const char* path) {
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

Path_Root* path_parser_parse_path(const char* path) {
    if (!path_parser_is_path_valid(path)) {
        return NULL;
    }

    uint8_t drive_no = get_digit(path[0]);
    char* rest_of_path = str_slice(path, 3, strlen(path));
    int separator_index = str_index_of(rest_of_path, '/');
    Path_Part* part = NULL;
    Path_Part* first_part = NULL;

    do {
        Path_Part* previous_part = part;
        part = malloc(sizeof(Path_Part));

        part->name = str_slice(rest_of_path, 0, separator_index);
        part->next = previous_part;

        if (previous_part) {
            previous_part->next = part;
        }

        if (!first_part) {
            first_part = part;
        }

        char* new_path = str_slice(rest_of_path, separator_index + 1, strlen(rest_of_path));

        free(rest_of_path);

        rest_of_path = new_path;

        separator_index = str_index_of(rest_of_path, '/');
    } while (separator_index >= 0);

    // TODO: figure out a way to squeeze the logic from while
    // instead of creating another branch here
    if (strlen(rest_of_path) > 0) {
        Path_Part* previous_part = part;
        part = malloc(sizeof(Path_Part));

        part->name = rest_of_path;
        part->next = NULL;

        previous_part->next = part;
    }

    Path_Root* root = malloc(sizeof(Path_Root));

    root->driver_no = drive_no;
    root->part = first_part;

    return root;
}

void path_parser_free_part(Path_Part* part) {
    if (!part) {
        return;
    }

    path_parser_free_part(part->next);

    free(part->name);
    free(part);
}

void path_parser_free_root(Path_Root* root) {
    if (!root) {
        return;
    }

    path_parser_free_part(root->part);

    free(root);
}
