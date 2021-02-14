#include "html_parser.h"

#define MAX_TOKEN_TEXT_LENGTH       256
#define MIN_HTML_CONTENT_LENGTH     7   // e.g. <a></a>
#define NEW_LINE_SEQUENCE_LENGTH    2   // \n
#define DIV_TAG_START_LENGTH        20  // <div class=\"root\"> = 20 chars
#define DIV_TAG_END_LENGTH          9   // \n<\/div>
#define SPAN_TAG_END_LENGTH         7   // </span>
#define SPAN_TAG_CLASS_OFFSET       13  // <span class="
#define A_TAG_HREF_OFFSET           10  // <a href="/
#define A_TAG_END_LENGTH            4   // </a>
#define HREF_PAGE_ID_LENGTH         3   // e.g. 100

static void next_token(char **cursor) {
    (*cursor) += 1;
}

static void next_n_token(char **cursor, size_t n) {
    (*cursor) += n;
}

static bool is_newline(char **cursor) {
    return (*cursor)[0] == '\\' && (*cursor)[1] == 'n';
}

/// @brief Checks if the cursor it at the start of a tag based on the first char
/// @param tag_char the first character in the tag, e.g. 's' for span
static bool is_start_of_tag(char **cursor, char tag_char) {
    return (*cursor)[0] == '<' && (*cursor)[1] == tag_char;
}

/// @brief Checks if the cursor it at the end of a tag based on the first char
/// @param tag_char the first character in the tag, e.g. 's' for span
static bool is_end_of_tag(char **cursor, char tag_char) {
    return (*cursor)[0] == '<' && (*cursor)[1] == '/' && (*cursor)[2] == tag_char;
}

static void set_token_text(page_token_t *token, char *str, size_t length) {
    token->length = length;
    token->text = calloc(token->length + 1, sizeof(char));
    strncpy(token->text, str, token->length);
    token->text[token->length] = '\0';
}

static void add_separator_token(page_t *page, char *str, size_t length, bool inherit_style) {
    page_token_t *token = page_token_create_empty();
    page_token_append(page, token, inherit_style);
    set_token_text(token, str, length);
}

/// @brief Removes unnecessary backslashes and div-tag
static bool clean_html_content(char *buf, const char *html_content, size_t size) {
    assert(buf != NULL);
    assert(html_content != NULL);

    int i = 0;
    int end = size;
    int buf_position = 0;

    // Check if the start is a div tag
    if (html_content[0] == '<' && html_content[1] == 'd') {
        i = DIV_TAG_START_LENGTH;
        end = size - DIV_TAG_END_LENGTH;
    }

    // Skip first and last characters to remove div-tag
    for (; i < end; i++, buf_position++) {
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

static bool parse_a_tag(page_t *page, char **cursor) {
    if (!is_start_of_tag(cursor, 'a')) {
        error_set_with_string(
            TTT_ERROR_HTML_PARSER_FAILED,
            "ERROR: Invalid HTML content string, expected a tag"
        );

        return false;
    }

    page_token_t *token = page_token_create_empty();
    page_token_append(page, token, true);

    // Go to start of href id
    next_n_token(cursor, A_TAG_HREF_OFFSET);

    char id_buf[HREF_PAGE_ID_LENGTH];

    // Assume that each link will only contain page ids with length HREF_PAGE_ID_LENGTH
    for (int i = 0; i < HREF_PAGE_ID_LENGTH; i++) {
        id_buf[i] = **cursor;
        next_token(cursor);
    }

    int id = atoi(id_buf);

    if (id == -1) {
        error_set_with_string(
            TTT_ERROR_HTML_PARSER_FAILED,
            "ERROR: Could not convert link href to page id"
        );

        return false;
    }

    token->href = id;
    token->type = PAGE_TOKEN_LINK;

    // Skip to end of opening tag
    while ((**cursor) != '>') {
        next_token(cursor);
    }

    // Go to first character inside tag
    next_token(cursor);

    // Extract link text
    int length = 0;
    char *text_start = *cursor;

    while ((**cursor) != '\0') {
        if (is_end_of_tag(cursor, 'a')) {
            set_token_text(token, text_start, length);
            break;
        }

        length++;
        next_token(cursor);
    }

    // Go to the character after the a tag
    next_n_token(cursor, A_TAG_END_LENGTH);

    return true;
}

static bool parse_whitespace(page_t *page, char **cursor, bool inherit_style) {
    // Buffer for storing any characters between spans
    int i = 0;
    char separator_buf[MAX_TOKEN_TEXT_LENGTH];

    while (**cursor != '<') {
        if (**cursor == '\0') {
            return false;
        } else if (is_newline(cursor)) {
            // Some spans will be separated by new line and possibly some content
            if (i != 0) {
                add_separator_token(page, separator_buf, i, inherit_style);
                i = 0;
                separator_buf[0] = '\0';
            }

            // Move to next non-newline character
            next_n_token(cursor, NEW_LINE_SEQUENCE_LENGTH);
        } else {
            separator_buf[i] = **cursor;
            i++;
            next_token(cursor);
        }
    }

    // When we find the end of the span tag, add the previous whitespace token (if any)
    if (i != 0) {
        add_separator_token(page, separator_buf, i, inherit_style);
    }

    return true;
}

// TODO: Some text will be in h1 tags instead of span for some reason
static bool parse_span_tag(page_t *page, char **cursor) {
    if (!is_start_of_tag(cursor, 's')) {
        error_set_with_string(
            TTT_ERROR_HTML_PARSER_FAILED,
            "ERROR: Invalid HTML content string, expected span tag"
        );

        return false;
    }

    page_token_t *token = page_token_create_empty();
    page_token_append(page, token, false);

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
            if (strncmp(text_buf, "toprow", 6) == 0) {
                token->type = PAGE_TOKEN_HEADER;
            } else if (strncmp(text_buf, "DH", 2) == 0) {
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
            if (i != 0) {
                // Save current token text before parsing the (possible) whitespace
                set_token_text(token, text_buf, i);

                // Reset buf so that we do not try and save the token text again
                // when we find the end of the span tag
                i = 0;
                text_buf[0] = '\0';
            }

            // Move to next non-newline character
            next_n_token(cursor, NEW_LINE_SEQUENCE_LENGTH);

            if (!parse_whitespace(page, cursor, true)) {
                return false;
            }
        } else if (is_start_of_tag(cursor, 'a')) {
            if (!parse_a_tag(page, cursor)) {
                return false;
            }
        } else if (is_end_of_tag(cursor, 's')) {
            if (i != 0) {
                set_token_text(token, text_buf, i);
            }

            next_n_token(cursor, SPAN_TAG_END_LENGTH);
            return true;
        } else {
            text_buf[i] = **cursor;
            i++;
            next_token(cursor);
        }
    }

    // The loop only exits on '\0' which should never happen in this function.
    // Instead, we handle it in the caller. This way we can differentiate between
    // an unexpected and expected end to the HTML content.
    return false;
}

void html_parser_get_page_tokens(page_t *page, const char *html, size_t size) {
    if (!html || size == 0) {
        error_set_with_string(
            TTT_ERROR_HTML_PARSER_FAILED,
            "ERROR: Could not parse empty HTML page content"
        );

        return;
    } else if (size < MIN_HTML_CONTENT_LENGTH) {
        error_set_with_string(
            TTT_ERROR_HTML_PARSER_FAILED,
            "ERROR: Could not parse too short HTML page content"
        );

        return;
    }

    char buf[size];
    char *cursor = buf;

    if (!clean_html_content(buf, html, size)) {
        return;
    }

    // The HTML content must start with a span or h1 tag
    if (!is_start_of_tag(&cursor, 's') && !is_start_of_tag(&cursor, 'h')) {
        error_set_with_string(
            TTT_ERROR_HTML_PARSER_FAILED,
            "ERROR: Invalid start tag in HTML content, expected <span> or <h1>"
        );

        return;
    }

    while ((*cursor) != '\0') {
        if (!parse_span_tag(page, &cursor)) {
            error_set_with_string(
                TTT_ERROR_HTML_PARSER_FAILED,
                "ERROR: Unexpected end to HTML content"
            );

            if (page->tokens) {
                page_tokens_destroy(page);
                page->tokens = NULL;
            }

            return;
        }

        parse_whitespace(page, &cursor, false);
    }
}
