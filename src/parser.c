#include "parser.h"

#define JSMN_PARENT_LINKS
#include "../lib/jsmn.h"

#define TOKENS_SIZE 256
#define ESCAPED_CHAR_SEQUENCE_LENGTH 5

static void next_token(jsmntok_t **cursor) {
    *cursor += 1;
}

/// @brief Moves a pointer to a token forward n steps
static void next_n_token(jsmntok_t **cursor, size_t n) {
    // TODO: Do this recursively to make sure that we are actually at the end of the array
    //       For example, if we have an object inside the array, we must move more than 'size' steps
    for (size_t i = 0; i < n; i++) {
        next_token(cursor);
    }
}

static size_t token_length(jsmntok_t *cursor) {
    if (!cursor) {
        return 0;
    }

    return cursor->end - cursor->start;
}

static bool string_is_literal_null(const char *str, size_t length) {
    return strncmp(str, "null", length) == 0;
}

static char *get_string(const char *data, jsmntok_t *cursor) {
    size_t length = token_length(cursor);

    if (!data || length == 0) {
        return NULL;
    }

    const char *str_start = data + cursor->start;

    // If the string is 'null' we assume that it is invalid
    if (length == 4 && strncmp(str_start, "null", length) == 0) {
        return NULL;
    }

    char *str = calloc(length + 1, sizeof(char));
    str[length] = '\0';
    strncpy(str, data + cursor->start, length);
    return str;
}

/// @brief Reads a token and converts it into a (positive) numerical value
/// @param data the raw JSON string
/// @param token the token to parse as a numeric value
/// @param max_value the max value for the numeric type
/// @return the numeric value or -1 if parsing failed or value is greater than max_value
static size_t get_unsigned_numeric(const char *data, jsmntok_t *cursor, size_t max_value) {
    size_t length = token_length(cursor);
    char *value = get_string(data, cursor);

    if (!value || string_is_literal_null(value, length)) {
        free(value);
        return -1;
    }

    int numeric = atoi(value);
    free(value);

    if (!numeric || numeric < 0 || numeric > max_value) {
        return -1;
    }

    return numeric;
}

static void parse_content(page_t *page, const char *data, jsmntok_t **cursor) {
    if ((*cursor)->type != JSMN_ARRAY) {
        error_set_with_string(
            TTT_ERROR_PAGE_PARSER_FAILED,
            "ERROR: Could not parse invalid page content data type"
        );
        return;
    }

    size_t array_size = (*cursor)->size;

    if (array_size == 0) {
        error_set_with_string(
            TTT_ERROR_PAGE_PARSER_FAILED,
            "ERROR: Could not parse empty page content array"
        );
        return;
    }

    // TODO: Add support for more than one element?
    //       For certain pages, there are a "sub" page with other data, e.g. the stock market page
    // Go to the first array element
    next_token(cursor);
    char *html = get_string(data, *cursor);
    html_parser_get_page_tokens(page, html, token_length(*cursor));
    next_n_token(cursor, array_size - 1);
    free(html);
}

static page_t *get_page(const char *data, jsmntok_t **cursor) {
    char *key = NULL;
    size_t keys = (*cursor)->size;

    // We need a minimum of 1 key to have a non-empty page object
    if (keys < 1) {
        error_set_with_string(
            TTT_ERROR_PAGE_PARSER_FAILED,
            "ERROR: Could not parse empty page object"
        );
        return NULL;
    }

    page_t *page = page_create_empty();

    for (size_t i = 0; i < keys; i++) {
        next_token(cursor);
        key = get_string(data, *cursor);
        next_token(cursor);

        if (!key) {
            continue;
        }

        if (strcmp(key, "num") == 0) {
            page->id = get_unsigned_numeric(data, *cursor, UINT16_MAX);
        } else if (strcmp(key, "prev_page") == 0) {
            page->prev_id = get_unsigned_numeric(data, *cursor, UINT16_MAX);
        } else if (strcmp(key, "next_page") == 0) {
            page->next_id = get_unsigned_numeric(data, *cursor, UINT16_MAX);
        } else if (strcmp(key, "date_updated_unix") == 0) {
            page->unix_date = get_unsigned_numeric(data, *cursor, SIZE_MAX);
        } else if (strcmp(key, "title") == 0) {
            page->title = get_string(data, *cursor);
        } else if (strcmp(key, "content") == 0) {
            parse_content(page, data, cursor);
        }

        free(key);
    }

    return page;
}

page_collection_t *parser_get_page_collection(const char *data, size_t size) {
    if (!data || *data == '\0' || size == 0) {
        error_set_with_string(
            TTT_ERROR_PAGE_PARSER_FAILED,
            "ERROR: Could not parse empty response data"
        );
        return NULL;
    }

    jsmn_parser parser;
    jsmn_init(&parser);
    jsmntok_t tokens[TOKENS_SIZE];
    size_t keys = jsmn_parse(&parser, data, size, tokens, sizeof(tokens) / sizeof(tokens[0]));

    if (keys < 3 || tokens[0].type != JSMN_ARRAY || tokens[1].type != JSMN_OBJECT) {
        error_set_with_string(
            TTT_ERROR_PAGE_PARSER_FAILED,
            "ERROR: Could not parse response data with invalid structure"
        );
        return NULL;
    }

    jsmntok_t *cursor = tokens;
    size_t parsed_objects = 0;
    size_t array_size = cursor->size;
    page_collection_t *collection = page_collection_create(array_size);

    for (size_t i = 0; i < array_size; i++) {
        next_token(&cursor);

        if (cursor->type == JSMN_OBJECT) {
            page_t *page = get_page(data, &cursor);

            if (page) {
                collection->pages[parsed_objects] = page;
                parsed_objects++;
            }
        }
    }

    if (array_size > parsed_objects) {
        if (parsed_objects == 0) {
            error_set_with_string(
                TTT_ERROR_PAGE_PARSER_FAILED,
                "ERROR: No pages could be parsed"
            );
            page_collection_destroy(collection);
            return NULL;
        }

        page_collection_resize(collection, parsed_objects);
    }

    return collection;
}
