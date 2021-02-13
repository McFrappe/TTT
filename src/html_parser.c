#include "html_parser.h"

#define MAX_TOKENS              256
#define DIV_TAG_START_LENGTH    20  // <div class=\"root\"> = 20 chars
#define DIV_TAG_END_LENGTH      9   // \n<\/div>
#define SPAN_TAG_END_LENGTH     7   // </span>
#define SPAN_TAG_CLASS_OFFSET   13  // <span class="

static void next_token(char **cursor) {
    (*cursor) += 1;
}

static void next_n_token(char **cursor, size_t n) {
    (*cursor) += n;
}

/// @brief Removes unnecessary backslashes and div-tag
static bool clean_content_junk(char *buf, const char *html_content, size_t size) {
    int buf_position = 0;

    // Make sure we dont read incorrectly or cause SIGSEGV
    if (!html_content || size <= 26) {
        return false;
    }

    // Skip first and last characters to remove div-tag
    for (int i = DIV_TAG_START_LENGTH; i < (size - DIV_TAG_END_LENGTH); i++, buf_position++) {
        // Only remove backslashes that are not part of the newline escape sequence
        if (html_content[i] == '\\' && html_content[i + 1] != 'n') {
            // Move to next non-backslash character
            i++;
        }

        buf[buf_position] = html_content[i];
    }

    buf[buf_position] = '\0';

    return true;
}

static bool parse_span_tag(page_t *page, char **cursor) {
    if ((*cursor)[0] != '<' || (*cursor)[1] != 's') {
        error_set_with_string(
            TTT_ERROR_HTML_PARSER_FAILED,
            "ERROR: Invalid HTML content string, expected span-tag"
        );

        return false;
    }

    page_token_t *token = page_token_create_empty();
    page_token_append(page, token);

    // Move to first character in span class attribute
    next_n_token(cursor, SPAN_TAG_CLASS_OFFSET);

    int i = 0;
    char class_buf[32];

    // Initialize buffer to prevent error if the HTML format is invalid
    class_buf[0] = '\0';

    // Extract each classname, separated by a space
    while (**cursor != '"') {
        if (**cursor == ' ' || (*cursor)[1] == '"') {
            // We have a complete classname in class_buf, add token attribute
            if (strncmp(class_buf, "DH", 2)) {
                token->style.extra = PAGE_TOKEN_ATTR_BOLD;
            } else if (strncmp(class_buf, "B", 1)) {
                token->style.fg = PAGE_TOKEN_ATTR_BLUE;
            } else if (strncmp(class_buf, "C", 1)) {
                token->style.fg = PAGE_TOKEN_ATTR_CYAN;
            } else if (strncmp(class_buf, "W", 1)) {
                token->style.fg = PAGE_TOKEN_ATTR_WHITE;
            } else if (strncmp(class_buf, "G", 1)) {
                token->style.fg = PAGE_TOKEN_ATTR_GREEN;
            } else if (strncmp(class_buf, "Y", 1)) {
                token->style.fg = PAGE_TOKEN_ATTR_YELLOW;
            } else if (strncmp(class_buf, "R", 1)) {
                token->style.fg = PAGE_TOKEN_ATTR_RED;
            } else if (strncmp(class_buf, "bgB", 3)) {
                token->style.bg = PAGE_TOKEN_ATTR_BG_BLUE;
            } else if (strncmp(class_buf, "bgC", 3)) {
                token->style.bg = PAGE_TOKEN_ATTR_BG_CYAN;
            } else if (strncmp(class_buf, "bgW", 3)) {
                token->style.bg = PAGE_TOKEN_ATTR_BG_WHITE;
            } else if (strncmp(class_buf, "bgG", 3)) {
                token->style.bg = PAGE_TOKEN_ATTR_BG_GREEN;
            } else if (strncmp(class_buf, "bgY", 3)) {
                token->style.bg = PAGE_TOKEN_ATTR_BG_YELLOW;
            } else if (strncmp(class_buf, "bgR", 3)) {
                token->style.bg = PAGE_TOKEN_ATTR_BG_RED;
            }

            // Reset class_buf
            i = 0;
        } else {
            class_buf[i] = **cursor;
            i++;
        }

        next_token(cursor);
    }

    // Move to first character inside the span-tag
    next_n_token(cursor, 2);

    // Move to the end of the span-tag
    while ((*cursor)[0] != '\0') {
        if ((*cursor)[0] == '<' && (*cursor)[1] == '/' && (*cursor)[2] == 's') {
            next_n_token(cursor, SPAN_TAG_END_LENGTH);
            break;
        }

        next_token(cursor);
    }

    //printf("next %c\n", **cursor);

    // TODO: Add text to token

    return true;
}

void html_parser_get_page_tokens(page_t *page, const char *html, size_t size) {
    if (!html || size == 0) {
        error_set_with_string(
            TTT_ERROR_HTML_PARSER_FAILED,
            "ERROR: Could not parse empty HTML page content"
        );

        return;
    }

    char buf[size];
    char *cursor = buf;

    if (!clean_content_junk(buf, html, size)) {
        return;
    }

    while (true) {
        if (!parse_span_tag(page, &cursor)) {
            break;
        }
    }
}
