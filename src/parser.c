#include "parser.h"
#include "../lib/jsmn.h"

#define TOKENS_SIZE 128
#define EXPECTED_OBJECT_SIZE 8

static char *get_string_value(const char *data, jsmntok_t token) {
    assert(data != NULL);

    size_t length = token.end - token.start;

    if (length == 0)
        return NULL;

    char *str = calloc(length + 1, sizeof(char));
    str[length] = '\0';
    strncpy(str, data + token.start, length);
    return str;
}

// TODO: Pass in the entire tokens array so that we can loop through all items
static post_content_t **get_content(const char *data, jsmntok_t token) {
    /* char *content_string; */

    post_content_t **content = calloc(token.size, sizeof(post_content_t*));

    for (size_t i = 0; i < token.size; i++) {
        /* content_string = get_string_value(data, token) */
    }

    return content;
}

static void parse_key_value(page_t *page, const char *data, jsmntok_t token, jsmntok_t next_token, size_t *index) {
    char *value = NULL;
    char *key = get_string_value(data, token);

    if (strcmp(key, "num") == 0) {
        value = get_string_value(data, next_token);
        uint16_t id = atoi(value);

        free(value);

        if (!id) {
            printf("Expected numeric value for key, but got string: %s\n", value);
            return;
        }

        page->id = id;
    } else if (strcmp(key, "prev_page") == 0) {
        value = get_string_value(data, next_token);
        uint16_t prev_id = atoi(value);

        free(value);

        if (!prev_id) {
            printf("Expected numeric value for key, but got string: %s\n", value);
            return;
        }

        page->prev_id = prev_id;
    } else if (strcmp(key, "next_page") == 0) {
        value = get_string_value(data, next_token);
        uint16_t next_id = atoi(value);

        free(value);

        if (!next_id) {
            printf("Expected numeric value for key, but got string: %s\n", value);
            return;
        }

        page->next_id = next_id;
    } else if (strcmp(key, "date_updated_unix") == 0) {
        value = get_string_value(data, next_token);
        uint64_t date = atoi(value);

        free(value);

        if (!date) {
            printf("Expected numeric value for key, but got string: %s\n", value);
            return;
        }

        page->unix_date = date;
    } else if (strcmp(key, "title") == 0) {
        page->title = get_string_value(data, next_token);
    } else if (strcmp(key, "content") == 0) {
        *index += next_token.size;
        page->content_size = next_token.size;
        page->content = get_content(data, next_token);
    }

    *index += 1;

    free(key);
}

// TODO: Fix memory leaks
static page_t *parse_object(const char *data, jsmntok_t obj, jsmntok_t *tokens, size_t *obj_index) {
    jsmntok_t cursor;
    page_t *page = calloc(1, sizeof(page_t));
    size_t iterations = 0;
    size_t i = *obj_index + 1;

    while (iterations < obj.size) {
        cursor = tokens[i];
        parse_key_value(page, data, cursor, tokens[i + 1], &i);
        iterations++;
        i++;
    }

    // TODO: Should we have -1?
    *obj_index = i - 1;

    return page;
}

page_collection_t *parser_convert_to_pages(const char *data, size_t size) {
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
    page_t **pages = calloc(array.size, sizeof(page_t*));

    for (size_t i = 1; i < keys; i++) {
        jsmntok_t token = tokens[i];

        if (token.type == JSMN_OBJECT) {
            if (token.size != EXPECTED_OBJECT_SIZE) {
                printf("Failed to parse JSON page object with invalid size: %d\n", token.size);
                continue;
            }

            page_t *page = parse_object(data, token, tokens, &i);

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
