#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct page page_t;
typedef struct post post_t;
typedef struct page_collection page_collection_t;
typedef struct post_content post_content_t;
typedef enum post_content_type {
    HEADLINE,
    ARTICLE
} post_content_type_t;

struct post_content {
    char *title;
    char *text;
    uint16_t id;
    post_content_type_t type;
};

struct page {
    uint16_t id, prev_id, next_id;
    uint64_t unix_date;
    char *title;
    size_t content_size;
    post_content_t **content;
};

struct page_collection {
    page_t **pages;
    size_t size;
};

page_collection_t *parser_convert_to_pages(const char *data, size_t size);
