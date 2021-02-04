#include "errors.h"

static const char *custom_error_string = NULL;

void error_set(ttt_error_t code) {
    errno = code;
    custom_error_string = NULL;
}

void error_set_with_string(ttt_error_t code, const char *str) {
    errno = code;
    custom_error_string = str;
}

bool error_is_set() {
    return errno != TTT_ERROR_NONE;
}

const char *error_get_string() {
    if (!error_is_set()) {
        return NULL;
    }

    if (custom_error_string) {
        return custom_error_string;
    }

    ttt_error_t code = errno;

    // Default error code messages
    switch (code) {
    case TTT_ERROR_OUT_OF_MEMORY:
        return "ERROR: Out of memory";

    case TTT_ERROR_REQUEST_FAILED:
        return "ERROR: HTTP request failed";

    case TTT_ERROR_PAGE_PARSER_FAILED:
        return "ERROR: Could not parse response";

    case TTT_ERROR_HTML_PARSER_FAILED:
        return "ERROR: Could not parse HTML page content";

    default:
        return "ERROR: Unknown";
    }
}

void error_reset() {
    error_set(TTT_ERROR_NONE);
}
