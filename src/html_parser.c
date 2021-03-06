#include "html_parser.h"

#define MAX_TOKEN_TEXT_LENGTH           256
#define MIN_HTML_CONTENT_LENGTH         7   // e.g. <a></a>
#define NEW_LINE_SEQUENCE_LENGTH        2   // \n
#define DIV_TAG_START_LENGTH            20  // <div class=\"root\"> = 20 chars
#define DIV_TAG_END_LENGTH              9   // \n<\/div>
#define SPAN_TAG_END_LENGTH             7   // </span>
#define SPAN_TAG_CLASS_OFFSET           13  // <span class="
#define A_TAG_HREF_OFFSET               10  // <a href="/
#define A_TAG_END_LENGTH                4   // </a>
#define HREF_PAGE_ID_LENGTH             3   // e.g. 100
#define HEADER_TAG_CLASS_OFFSET         11  // <h1 class="
#define HEADER_TAG_END_LENGTH           5   // </h1>
#define UNICODE_ESCAPE_SEQUENCE_LENGTH  6   // e.g. \u00f6
#define MAX_HTML_ESCAPE_SEQUENCE_LENGTH 26  // e.g. &ClockwiseContourIntegral;

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

static void save_current_text_to_token(page_t *page, page_token_t *token, char *buf, int *i, bool *create_new) {
    if (*i != 0) {
        if (*create_new) {
            token = page_token_create_empty();
            page_token_append(page, token, true);
            (*create_new) = false;
        }

        set_token_text(token, buf, *i);
        (*i) = 0;
        buf[0] = '\0';
    }

    (*create_new) = true;
}

static void add_separator_token(page_t *page, char *str, size_t length, bool inherit_style) {
    page_token_t *token = page_token_create_empty();
    page_token_append(page, token, inherit_style);
    set_token_text(token, str, length);
}

static void replace_unicode_escape_sequence(const char *html_content, int *start_index, char **dest, int *dest_position) {
    const char *sequence_start = html_content + (*start_index);

    if (
        strncmp(sequence_start, "\\u00e4", UNICODE_ESCAPE_SEQUENCE_LENGTH) == 0 ||
        strncmp(sequence_start, "\\u00e5", UNICODE_ESCAPE_SEQUENCE_LENGTH) == 0
    ) {
        (*dest)[*dest_position] = 'a';
    } else if (
        strncmp(sequence_start, "\\u00c4", UNICODE_ESCAPE_SEQUENCE_LENGTH) == 0 ||
        strncmp(sequence_start, "\\u00c5", UNICODE_ESCAPE_SEQUENCE_LENGTH) == 0
    ) {
        (*dest)[*dest_position] = 'A';
    } else if (strncmp(sequence_start, "\\u00f6", UNICODE_ESCAPE_SEQUENCE_LENGTH) == 0) {
        (*dest)[*dest_position] = 'o';
    } else if (strncmp(sequence_start, "\\u00d6", UNICODE_ESCAPE_SEQUENCE_LENGTH) == 0) {
        (*dest)[*dest_position] = 'O';
    } else if (strncmp(sequence_start, "\\u00e9", UNICODE_ESCAPE_SEQUENCE_LENGTH) == 0) {
        (*dest)[*dest_position] = 'e';
    } else {
        char error[256];
        snprintf(error, 256, "ERROR: Found unhandled unicode escape sequence: %.6s", sequence_start);
        error_set_with_string(
            TTT_ERROR_HTML_PARSER_FAILED,
            error
        );
        // There is no need to die if we found an unhandled escape sequence,
        // but we should make sure to show an error so that it can be fixed
        (*dest)[*dest_position] = 'X';
    }

    // Position the cursor on the last character of the escape sequence
    (*start_index) += UNICODE_ESCAPE_SEQUENCE_LENGTH - 1;
    (*dest_position) += 1;
}

static void replace_html_escape_sequence(const char *html_content, int *start_index, char **dest, int *dest_position) {
    const char *sequence_start = html_content + (*start_index);
    int i = 0;
    char buf[MAX_HTML_ESCAPE_SEQUENCE_LENGTH];

    while (sequence_start[i] != ';' && i < MAX_HTML_ESCAPE_SEQUENCE_LENGTH) {
        buf[i] = sequence_start[i];
        i++;
    }

    buf[i] = ';';
    i++;
    buf[i] = '\0';

    if (strcmp(buf, "&amp;") == 0) {
        (*dest)[*dest_position] = '&';
    } else if (strcmp(buf, "&lt;") == 0) {
        (*dest)[*dest_position] = '<';
    } else if (strcmp(buf, "&gt;") == 0) {
        (*dest)[*dest_position] = '>';
    } else {
        char error[256];
        snprintf(error, 256, "ERROR: Found unhandled html escape sequence: %s", buf);
        error_set_with_string(
            TTT_ERROR_HTML_PARSER_FAILED,
            error
        );
        (*dest)[*dest_position] = 'X';
    }

    // Position the cursor on the last character of the escape sequence
    (*start_index) += i - 1;
    (*dest_position) += 1;
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
    for (; i < end; i++) {
        // Only remove backslashes that are not part of the newline escape sequence
        if (html_content[i] == '\\' && html_content[i + 1] != 'n') {
            if (html_content[i + 1] == 'u') {
                replace_unicode_escape_sequence(html_content, &i, &buf, &buf_position);
                continue;
            } else {
                i++;
            }
        } else if (html_content[i] == '&' && html_content[i + 1] != ' ') {
            replace_html_escape_sequence(html_content, &i, &buf, &buf_position);
            continue;
        }

        buf[buf_position] = html_content[i];
        buf_position++;
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
    char id_buf[HREF_PAGE_ID_LENGTH + 1];

    // Assume that each link will only contain page ids with length HREF_PAGE_ID_LENGTH
    for (int i = 0; i < HREF_PAGE_ID_LENGTH; i++) {
        id_buf[i] = **cursor;
        next_token(cursor);
    }

    id_buf[HREF_PAGE_ID_LENGTH] = '\0';
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
static bool parse_tag(page_t *page, char **cursor) {
    bool is_span = is_start_of_tag(cursor, 's');
    bool is_header = is_start_of_tag(cursor, 'h');

    if (!is_span && !is_header) {
        error_set_with_string(
            TTT_ERROR_HTML_PARSER_FAILED,
            "ERROR: Invalid HTML content string, expected <span> or <h1> tag"
        );
        return false;
    }

    page_token_t *token = page_token_create_empty();
    page_token_append(page, token, false);

    // Move to first character in class attribute
    if (is_span) {
        next_n_token(cursor, SPAN_TAG_CLASS_OFFSET);
    } else {
        next_n_token(cursor, HEADER_TAG_CLASS_OFFSET);
    }

    int i = 0;
    char text_buf[MAX_TOKEN_TEXT_LENGTH];
    // Initialize buffer to prevent error if the HTML format is invalid
    text_buf[0] = '\0';

    // Extract each classname, separated by a space
    while (**cursor != '>') {
        if (**cursor == ' ' || **cursor == '"' || **cursor == '\'') {
            // Terminate buffer to prevent overflow in strcmp
            text_buf[i] = '\0';

            // We have a complete classname in text_buf, add token attribute
            if (strcmp(text_buf, "toprow") == 0) {
                token->type = PAGE_TOKEN_HEADER;
            } else if (strcmp(text_buf, "DH") == 0) {
                token->style.extra = PAGE_TOKEN_ATTR_BOLD;
            } else if (strcmp(text_buf, "B") == 0) {
                token->style.fg = PAGE_TOKEN_ATTR_BLUE;
            } else if (strcmp(text_buf, "C") == 0) {
                token->style.fg = PAGE_TOKEN_ATTR_CYAN;
            } else if (strcmp(text_buf, "W") == 0) {
                token->style.fg = PAGE_TOKEN_ATTR_WHITE;
            } else if (strcmp(text_buf, "G") == 0) {
                token->style.fg = PAGE_TOKEN_ATTR_GREEN;
            } else if (strcmp(text_buf, "Y") == 0) {
                token->style.fg = PAGE_TOKEN_ATTR_YELLOW;
            } else if (strcmp(text_buf, "R") == 0) {
                token->style.fg = PAGE_TOKEN_ATTR_RED;
            } else if (strcmp(text_buf, "bgB") == 0) {
                token->style.bg = PAGE_TOKEN_ATTR_BG_BLUE;
            } else if (strcmp(text_buf, "bgC") == 0) {
                token->style.bg = PAGE_TOKEN_ATTR_BG_CYAN;
            } else if (strcmp(text_buf, "bgW") == 0) {
                token->style.bg = PAGE_TOKEN_ATTR_BG_WHITE;
            } else if (strcmp(text_buf, "bgG") == 0) {
                token->style.bg = PAGE_TOKEN_ATTR_BG_GREEN;
            } else if (strcmp(text_buf, "bgY") == 0) {
                token->style.bg = PAGE_TOKEN_ATTR_BG_YELLOW;
            } else if (strcmp(text_buf, "bgR") == 0) {
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
    bool should_create_new_token = false;

    // Move to the end of the span-tag
    while (**cursor != '\0') {
        if (is_newline(cursor)) {
            save_current_text_to_token(page, token, text_buf, &i, &should_create_new_token);
            // Move to next non-newline character
            next_n_token(cursor, NEW_LINE_SEQUENCE_LENGTH);

            if (!parse_whitespace(page, cursor, true)) {
                return false;
            }
        } else if (is_start_of_tag(cursor, 'a')) {
            save_current_text_to_token(page, token, text_buf, &i, &should_create_new_token);

            if (!parse_a_tag(page, cursor)) {
                return false;
            }
        } else if ((is_span && is_end_of_tag(cursor, 's')) || (is_header && is_end_of_tag(cursor, 'h'))) {
            save_current_text_to_token(page, token, text_buf, &i, &should_create_new_token);

            if (is_span) {
                next_n_token(cursor, SPAN_TAG_END_LENGTH);
            } else {
                next_n_token(cursor, HEADER_TAG_END_LENGTH);
            }

            return true;
        } else if (is_start_of_tag(cursor, 's')) {
            // For some reason, there might be a random span inside another span
            // that does not even close (?). Dont know what the fuck that is about.
            // You can see a full example of this in 'test/data/nested_span.html'
            while (**cursor != '>') {
                next_token(cursor);
            }

            // Go to first non-tag character
            next_token(cursor);
        } else {
            text_buf[i] = **cursor;
            i++;
            next_token(cursor);
        }
    }

    error_set_with_string(
        TTT_ERROR_HTML_PARSER_FAILED,
        "ERROR: Unexpected end to HTML content"
    );
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
        if (!parse_tag(page, &cursor)) {
            if (page->tokens) {
                page_tokens_destroy(page);
                page->tokens = NULL;
            }

            return;
        }

        parse_whitespace(page, &cursor, false);
    }
}
