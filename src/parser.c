#include "parser.h"
#include <string.h>
#include "../lib/jsmn.h"

#define TOKENS_SIZE 128
#define EXPECTED_OBJECT_SIZE 8

static page_t *parse_object(const char *data, jsmntok_t obj, jsmntok_t *tokens, size_t obj_index) {
    jsmntok_t cursor;
    page_t *page = calloc(1, sizeof(page_t));

    for (size_t i = 1; i <= obj.size; i++) {
        cursor = tokens[i + obj_index];

        if (cursor.type == JSMN_STRING) {
            printf("PAGE - String: %d\n", cursor.size);
        } else if (cursor.type == JSMN_ARRAY) {
            printf("PAGE - Array size: %d\n", cursor.size);
        } else if (cursor.type == JSMN_PRIMITIVE) {
            printf("PAGE - Primitive: %d\n", cursor.size);
        } else {
            printf("Found unhandled token type in page: %d\n", cursor.type);
            continue;
        }
    }

    *page = (page_t) {
        .id = 0,
        .prev_id = 0,
        .next_id = 0,
        .unix_date = 0,
        .title = "Hello from parser",
        .content = NULL,
    };

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

    jsmntok_t array = tokens[0];
    printf("JSON array size: %d\n", array.size);

    size_t parsed_objects = 0;
    page_t **pages = calloc(array.size, sizeof(page_t*));

    for (size_t i = 1; i < keys; i++) {
        jsmntok_t token = tokens[i];

        printf("Index: %zd\n", i);
        if (token.type == JSMN_OBJECT) {
            if (token.size != EXPECTED_OBJECT_SIZE) {
                printf("Failed to parse JSON page object with invalid size: %d\n", token.size);
                continue;
            }

            page_t *page = parse_object(data, token, tokens, i);

            // Move forward to the first token after the current page object
            i += token.size;

            if (page) {
                pages[parsed_objects] = page;
                parsed_objects++;
            }
        } else {
            printf("Key type: %d\n", token.type);
        }
    }

    page_collection_t *collection = calloc(1, sizeof(page_collection_t));
    collection->pages = pages;
    collection->size = parsed_objects;

    return collection;
}
