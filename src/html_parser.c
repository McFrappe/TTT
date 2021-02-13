#include "html_parser.h"


static void clean_content_junk() {
    
}

page_token_t **html_parser_get_page_tokens(const char *html, size_t size) {
    if (!html || size == 0) {
        error_set_with_string(
            TTT_ERROR_HTML_PARSER_FAILED,
            "ERROR: Could not parse empty HTML page content"
        );
        return NULL;
    }

    return NULL;
}