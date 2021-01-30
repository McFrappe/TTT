#include "parser.h"
#include "../lib/jsmn.h"

#define TOKENS_SIZE 256
#define ESCAPED_CHAR_SEQUENCE_LENGTH 5

static size_t calculate_token_length(jsmntok_t token) {
    return token.end - token.start;
}

static char *parser_get_unicode_string(const char *data, jsmntok_t token) {
    size_t length = calculate_token_length(token);

    if (!data || length == 0) {
        return NULL;
    }

    char buf[length];
    char sequence_buf[ESCAPED_CHAR_SEQUENCE_LENGTH];
    size_t buf_position = 0;

    for (size_t i = token.start; i < token.start + length; i++, buf_position++) {
        // Only search for escape sequences with the format \uXXXX
        if (
            data[i] == '\\' &&
            (i + ESCAPED_CHAR_SEQUENCE_LENGTH) <= length &&
            data[i + 1] == 'u'
        ) {
            for (size_t j = 0; j < ESCAPED_CHAR_SEQUENCE_LENGTH; j++ ) {
                sequence_buf[j] = data[i + j + 1];
            }

            i += ESCAPED_CHAR_SEQUENCE_LENGTH;

            if (
                strncmp(sequence_buf, "u00e4", ESCAPED_CHAR_SEQUENCE_LENGTH) == 0 ||
                strncmp(sequence_buf, "u00e5", ESCAPED_CHAR_SEQUENCE_LENGTH) == 0
            ) {
                buf[buf_position] = 'a';
            } else if (
                strncmp(sequence_buf, "u00c4", ESCAPED_CHAR_SEQUENCE_LENGTH) == 0 ||
                strncmp(sequence_buf, "u00c5", ESCAPED_CHAR_SEQUENCE_LENGTH) == 0
            ) {
                buf[buf_position] = 'A';
            } else if (strncmp(sequence_buf, "u00f6", ESCAPED_CHAR_SEQUENCE_LENGTH) == 0) {
                buf[buf_position] = 'o';
            } else if (strncmp(sequence_buf, "u00d6", ESCAPED_CHAR_SEQUENCE_LENGTH) == 0) {
                buf[buf_position] = 'O';
            } else {
                printf("Found unhandled unicode escape sequence: %s\n", sequence_buf);
            }
        } else {
            buf[buf_position] = data[i];
        }
    }

    // Removing escape sequences means that the string will shrink in size
    // so we use buf_position as length
    char *escaped = calloc(buf_position + 1, sizeof(char));
    strncpy(escaped, buf, buf_position);
    escaped[buf_position] = '\0';
    return escaped;
}

static char *parser_get_string(const char *data, jsmntok_t token) {
    size_t length = calculate_token_length(token);

    if (!data || length == 0) {
        return NULL;
    }

    char *str = calloc(length + 1, sizeof(char));
    str[length] = '\0';
    strncpy(str, data + token.start, length);
    return str;
}

/// @brief Reads a token and converts it into a (positive) numerical value
/// @param data the raw JSON string
/// @param token the token to parse as a numeric value
/// @param max_value the max value for the numeric type
/// @return the numeric value or -1 if parsing failed or value is greater than max_value
static size_t parser_get_unsigned_numeric(const char *data, jsmntok_t token, size_t max_value) {
    char *value = parser_get_string(data, token);

    if (!value) {
        return -1;
    }

    int numeric = atoi(value);
    free(value);

    if (!numeric) {
        printf("Expected numeric value for token, but got: %s\n", value);
        return -1;
    }

    if (numeric < 0) {
        printf("Expected positive numeric value: %d\n", numeric);
        return -1;
    }

    if (numeric > max_value) {
        printf("Expected numeric value smaller than max value: %d\n", numeric);
        return -1;
    }

    return numeric;
}

page_content_t *parser_get_page_content(const char *content, size_t size) {
    printf("Content data: %s\n", content);
    return NULL;
}

// TODO: Pass in the entire tokens array so that we can loop through all items
static page_content_t **parser_get_page_contents(const char *data, jsmntok_t token) {
    /* char *content_string; */
    page_content_t **content = calloc(token.size, sizeof(page_content_t *));

    for (size_t i = 0; i < token.size; i++) {
        /* content_string = parser_get_string(data, token) */
    }

    return content;
}

static page_t *parse_page(const char *data, jsmntok_t obj, jsmntok_t *tokens, size_t *obj_token_index) {
    jsmntok_t cursor;
    jsmntok_t next_token;
    page_t *page = calloc(1, sizeof(page_t));
    size_t iterations = 0;
    size_t i = *obj_token_index + 1;
    char *key = NULL;

    while (iterations < obj.size) {
        cursor = tokens[i];
        next_token = tokens[i + 1];
        key = parser_get_string(data, cursor);

        if (strcmp(key, "num") == 0) {
            page->id = parser_get_unsigned_numeric(data, next_token, UINT16_MAX);
        } else if (strcmp(key, "prev_page") == 0) {
            page->prev_id = parser_get_unsigned_numeric(data, next_token, UINT16_MAX);
        } else if (strcmp(key, "next_page") == 0) {
            page->prev_id = parser_get_unsigned_numeric(data, next_token, UINT16_MAX);
        } else if (strcmp(key, "date_updated_unix") == 0) {
            page->unix_date = parser_get_unsigned_numeric(data, next_token, SIZE_MAX);
        } else if (strcmp(key, "title") == 0) {
            page->title = parser_get_unicode_string(data, next_token);
        } else if (strcmp(key, "content") == 0) {
            page->content = parser_get_page_contents(data, next_token);
            page->content_size = next_token.size;
            i += next_token.size;
        }

        free(key);
        iterations++;
        i += 2;
    }

    *obj_token_index = i;
    return page;
}

page_collection_t *parser_get_page_collection(const char *data, size_t size) {
    if (!data || size == 0) {
        return NULL;
    }

    jsmn_parser parser;
    jsmn_init(&parser);
    jsmntok_t tokens[TOKENS_SIZE];
    size_t keys = jsmn_parse(&parser, data, size, tokens, sizeof(tokens) / sizeof(tokens[0]));

    if (keys < 0) {
        printf("Failed to parse response body: %ld\n", keys);
        return NULL;
    }

    if (keys < 1 || tokens[0].type != JSMN_ARRAY) {
        printf("Parser expected array at top level!\n");
        return NULL;
    }

    size_t parsed_objects = 0;
    jsmntok_t array = tokens[0];
    page_t **pages = calloc(array.size, sizeof(page_t *));

    for (size_t i = 1; i < keys; i++) {
        jsmntok_t token = tokens[i];

        if (token.type == JSMN_OBJECT) {
            page_t *page = parse_page(data, token, tokens, &i);

            if (page) {
                pages[parsed_objects] = page;
                parsed_objects++;
            }
        }
    }

    page_collection_t *collection = calloc(1, sizeof(page_collection_t));
    collection->pages = pages;
    collection->size = parsed_objects;
    return collection;
}
