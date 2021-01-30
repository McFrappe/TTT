#include "parser.h"

#define JSMN_PARENT_LINKS
#include "../lib/jsmn.h"

#define TOKENS_SIZE 256
#define ESCAPED_CHAR_SEQUENCE_LENGTH 5

static size_t calculate_token_length(jsmntok_t *cursor) {
    if (!cursor) {
        return 0;
    }

    return cursor->end - cursor->start;
}

/// @brief Moves a pointer to a token forward n steps
static void move_forward(jsmntok_t **cursor, size_t steps) {
    // TODO: Do this recursively to make sure that we are actually at the end of the array
    //       For example, if we have an object inside the array, we must move more than 'size' steps
    for (size_t i = 0; i < steps; i++) {
        *cursor += 1;
    }
}

static char *get_unicode_string(const char *data, jsmntok_t *cursor) {
    size_t length = calculate_token_length(cursor);

    if (!data || length == 0) {
        return NULL;
    }

    char buf[length];
    char sequence_buf[ESCAPED_CHAR_SEQUENCE_LENGTH];
    size_t buf_position = 0;
    size_t start_index = cursor->start;
    size_t end_index = cursor->end;

    for (size_t i = start_index; i < end_index; i++, buf_position++) {
        // Only search for escape sequences with the format \uXXXX
        if (
            data[i] == '\\' &&
            (i + ESCAPED_CHAR_SEQUENCE_LENGTH) < end_index &&
            data[i + 1] == 'u'
        ) {
            for (size_t j = 0; j < ESCAPED_CHAR_SEQUENCE_LENGTH; j++) {
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

static char *get_string(const char *data, jsmntok_t *cursor) {
    size_t length = calculate_token_length(cursor);

    if (!data || length == 0) {
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
    char *value = get_string(data, cursor);

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
    if (!content || size == 0) {
        return NULL;
    }

    return NULL;
}

static page_content_t *parse_page_content(const char *data, jsmntok_t **cursor) {
    if ((*cursor)->type != JSMN_ARRAY) {
        return NULL;
    }

    size_t array_size = (*cursor)->size;

    if (array_size == 0) {
        return NULL;
    }

    // Go to the first array element
    *cursor += 1;
    char *content_string = get_string(data, *cursor);
    page_content_t *content = parser_get_page_content(
                                  content_string,
                                  calculate_token_length(*cursor)
                              );
    move_forward(cursor, array_size - 1);
    free(content_string);
    return content;
}

static page_t *get_page(const char *data, jsmntok_t **cursor) {
    page_t *page = calloc(1, sizeof(page_t));
    char *key = NULL;
    size_t keys = (*cursor)->size;

    for (size_t i = 0; i < keys; i++) {
        *cursor += 1;
        key = get_string(data, *cursor);
        *cursor += 1;

        if (strcmp(key, "num") == 0) {
            page->id = get_unsigned_numeric(data, *cursor, UINT16_MAX);
        } else if (strcmp(key, "prev_page") == 0) {
            page->prev_id = get_unsigned_numeric(data, *cursor, UINT16_MAX);
        } else if (strcmp(key, "next_page") == 0) {
            page->next_id = get_unsigned_numeric(data, *cursor, UINT16_MAX);
        } else if (strcmp(key, "date_updated_unix") == 0) {
            page->unix_date = get_unsigned_numeric(data, *cursor, SIZE_MAX);
        } else if (strcmp(key, "title") == 0) {
            page->title = get_unicode_string(data, *cursor);
        } else if (strcmp(key, "content") == 0) {
            page->content = parse_page_content(data, cursor);
        }

        free(key);
    }

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

    if (keys < 3 || tokens[0].type != JSMN_ARRAY || tokens[1].type != JSMN_OBJECT) {
        return NULL;
    }

    jsmntok_t *cursor = tokens;
    size_t array_size = cursor->size;
    page_t **pages = calloc(array_size, sizeof(page_t *));
    size_t parsed_objects = 0;

    for (size_t i = 0; i < array_size; i++) {
        cursor++;

        if (cursor->type == JSMN_OBJECT) {
            page_t *page = get_page(data, &cursor);

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
