#pragma once
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

typedef struct page page_t;
typedef struct page_collection page_collection_t;
typedef struct page_content page_content_t;
typedef enum page_content_type_t {
    HEADLINE,
    ARTICLE
} page_content_type_t;

struct page_content {
    char *title;
    char *text;
    uint16_t id;
    page_content_type_t type;
};

struct page {
    uint16_t id, prev_id, next_id;
    uint64_t unix_date;
    char *title;
    page_content_t *content;
};

struct page_collection {
    page_t **pages;
    size_t size;
};

page_t *page_create_empty();
page_collection_t *page_collection_create(size_t size);
bool page_is_empty(page_t *page);
void page_collection_resize(page_collection_t *collection, size_t new_size);
void page_destroy(page_t *page);
void page_content_destroy(page_content_t *content);
void page_collection_destroy(page_collection_t *collection);
void page_collection_print(page_collection_t *collection, const char *name);
