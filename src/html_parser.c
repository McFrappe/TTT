#include "html_parser.h"

#define MAX_TOKENS                  256
#define MAX_TOKEN_TEXT_LENGTH       256
#define NEW_LINE_SEQUENCE_LENGTH    2   // \n
#define DIV_TAG_START_LENGTH        20  // <div class=\"root\"> = 20 chars
#define DIV_TAG_END_LENGTH          9   // \n<\/div>
#define SPAN_TAG_END_LENGTH         7   // </span>
#define SPAN_TAG_CLASS_OFFSET       13  // <span class="

static void next_token(char **cursor) {
    (*cursor) += 1;
}

static void next_n_token(char **cursor, size_t n) {
    (*cursor) += n;
}

static bool is_newline(char **cursor) {
    return (*cursor)[0] == '\\' && (*cursor)[1] == 'n';
}

static void set_token_text(page_token_t *token, const char *str, size_t length) {
    token->length = length;
    token->text = calloc(token->length + 1, sizeof(char));
    strncpy(token->text, str, token->length);
    token->text[token->length] = '\0';
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
    char text_buf[MAX_TOKEN_TEXT_LENGTH];

    // Initialize buffer to prevent error if the HTML format is invalid
    text_buf[0] = '\0';

    // Extract each classname, separated by a space
    while (**cursor != '>') {
        if (**cursor == ' ' || **cursor == '"') {
            // We have a complete classname in text_buf, add token attribute
            if (strncmp(text_buf, "DH", 2) == 0) {
                token->style.extra = PAGE_TOKEN_ATTR_BOLD;
            } else if (strncmp(text_buf, "B", 1) == 0) {
                token->style.fg = PAGE_TOKEN_ATTR_BLUE;
            } else if (strncmp(text_buf, "C", 1) == 0) {
                token->style.fg = PAGE_TOKEN_ATTR_CYAN;
            } else if (strncmp(text_buf, "W", 1) == 0) {
                token->style.fg = PAGE_TOKEN_ATTR_WHITE;
            } else if (strncmp(text_buf, "G", 1) == 0) {
                token->style.fg = PAGE_TOKEN_ATTR_GREEN;
            } else if (strncmp(text_buf, "Y", 1) == 0) {
                token->style.fg = PAGE_TOKEN_ATTR_YELLOW;
            } else if (strncmp(text_buf, "R", 1) == 0) {
                token->style.fg = PAGE_TOKEN_ATTR_RED;
            } else if (strncmp(text_buf, "bgB", 3) == 0) {
                token->style.bg = PAGE_TOKEN_ATTR_BG_BLUE;
            } else if (strncmp(text_buf, "bgC", 3) == 0) {
                token->style.bg = PAGE_TOKEN_ATTR_BG_CYAN;
            } else if (strncmp(text_buf, "bgW", 3) == 0) {
                token->style.bg = PAGE_TOKEN_ATTR_BG_WHITE;
            } else if (strncmp(text_buf, "bgG", 3) == 0) {
                token->style.bg = PAGE_TOKEN_ATTR_BG_GREEN;
            } else if (strncmp(text_buf, "bgY", 3) == 0) {
                token->style.bg = PAGE_TOKEN_ATTR_BG_YELLOW;
            } else if (strncmp(text_buf, "bgR", 3) == 0) {
                token->style.bg = PAGE_TOKEN_ATTR_BG_RED;
            }

            // Reset buf since there might be more classes
            i = 0;
            text_buf[0] = '\0';
        } else {
            text_buf[i] = **cursor;
            i++;
        }

        next_token(cursor);
    }

    // Move to first character inside the span-tag
    next_token(cursor);

    // Move to the end of the span-tag
    while ((*cursor)[0] != '\0') {
        if (is_newline(cursor)) {
            // TODO: We might need to handle new lines since there will be content after?
            next_n_token(cursor, NEW_LINE_SEQUENCE_LENGTH);
        } else if ((*cursor)[0] == '<' && (*cursor)[1] == '/' && (*cursor)[2] == 's') {
            set_token_text(token, text_buf, i);
            next_n_token(cursor, SPAN_TAG_END_LENGTH);
            break;
        } else {
            text_buf[i] = **cursor;
            i++;
            next_token(cursor);
        }
    }

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

        // Some spans will be separated by new line and possibly some content.
        // Make sure to skip this before trying to parse a span.
        if (*cursor != '\0' && is_newline(&cursor)) {
            while (*cursor != '<') {
                if (*cursor == '\0') {
                    return;
                }

                next_token(&cursor);
            }
        }
    }
}
