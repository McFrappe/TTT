#pragma once
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "shared.h"

typedef struct page page_t;
typedef struct page_token page_token_t;
typedef struct page_token_style page_token_style_t;
typedef struct page_collection page_collection_t;

typedef enum page_token_attr {
    PAGE_TOKEN_ATTR_BOLD,       // .DH
    PAGE_TOKEN_ATTR_BLUE,       // .B
    PAGE_TOKEN_ATTR_CYAN,       // .C
    PAGE_TOKEN_ATTR_WHITE,      // .W
    PAGE_TOKEN_ATTR_GREEN,      // .G
    PAGE_TOKEN_ATTR_YELLOW,     // .Y
    PAGE_TOKEN_ATTR_RED,        // .R
    PAGE_TOKEN_ATTR_BG_BLACK,   // default
    PAGE_TOKEN_ATTR_BG_BLUE,    // .bgB
    PAGE_TOKEN_ATTR_BG_CYAN,    // .bgC
    PAGE_TOKEN_ATTR_BG_WHITE,   // .bgW
    PAGE_TOKEN_ATTR_BG_GREEN,   // .bgG
    PAGE_TOKEN_ATTR_BG_YELLOW,  // .bgY
    PAGE_TOKEN_ATTR_BG_RED,     // .bgR
    PAGE_TOKEN_ATTR_NONE = -1
} page_token_attr_t;

typedef enum page_token_type {
    PAGE_TOKEN_HEADER,          // .toprow
    PAGE_TOKEN_TEXT,            // any text content inside tags
    PAGE_TOKEN_LINK             // <a>
} page_token_type_t;

struct page_token_style {
    page_token_attr_t fg;
    page_token_attr_t bg;
    page_token_attr_t extra;
};

struct page_token {
    char *text;
    uint16_t href;
    uint8_t length;
    page_token_type_t type;
    page_token_style_t style;
    page_token_t *next;
};

struct page {
    char *title;
    uint16_t id, prev_id, next_id;
    uint64_t unix_date;
    page_token_t *tokens;
    page_token_t *last_token;
};

struct page_collection {
    page_t **pages;
    size_t size;
};

page_t *page_create_empty();
page_token_t *page_token_create_empty();
page_collection_t *page_collection_create(size_t size);
bool page_is_empty(page_t *page);
void page_collection_resize(page_collection_t *collection, size_t new_size);
void page_destroy(page_t *page);
void page_tokens_destroy(page_t *page);
void page_token_append(page_t *page, page_token_t *token, bool inherit_style);
void page_collection_destroy(page_collection_t *collection);
void page_print(page_t *page);
void page_tokens_print(page_t *page);
void page_collection_print(page_collection_t *collection);
