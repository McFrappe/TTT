#pragma once
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>

extern int errno;

typedef enum ttt_error {
    TTT_ERROR_NONE,
    TTT_ERROR_OUT_OF_MEMORY,
    TTT_ERROR_REQUEST_FAILED,
    TTT_ERROR_PAGE_PARSER_FAILED,
    TTT_ERROR_HTML_PARSER_FAILED,
} ttt_error_t;

bool error_is_set();
void error_set(ttt_error_t code);
void error_set_with_string(ttt_error_t code, const char *str);
ttt_error_t error_get();
const char *error_get_string();
void error_reset();
