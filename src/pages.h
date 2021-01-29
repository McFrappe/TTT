#pragma once
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>

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
    size_t content_size;
    page_content_t **content;
};

struct page_collection {
    page_t **pages;
    size_t size;
};

void page_destroy(page_t *page);
void page_collection_print(page_collection_t *collection, const char *name);
void page_collection_destroy(page_collection_t *collection);
