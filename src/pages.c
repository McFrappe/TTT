#include "pages.h"

static page_t empty_page = {
    .id = -1,
    .prev_id = -1,
    .next_id = -1,
    .unix_date = -1,
    .title = NULL,
    .tokens = NULL
};

page_t *page_create_empty() {
    page_t *page = calloc(1, sizeof(page_t));

    if (!page) {
        return NULL;
    }

    memcpy(page, &empty_page, sizeof(empty_page));
    return page;
}

page_collection_t *page_collection_create(size_t size) {
    page_collection_t *collection = calloc(1, sizeof(page_collection_t));

    if (size == 0) {
        collection->pages = NULL;
    } else {
        collection->pages = calloc(size, sizeof(page_t *));
    }

    collection->size = size;
    return collection;
}

void page_collection_resize(page_collection_t *collection, size_t new_size) {
    if (!collection || collection->size == new_size) {
        return;
    }

    page_t **pages = realloc(collection->pages, sizeof(page_t *) * new_size);

    if (!pages) {
        return;
    }

    collection->pages = pages;
    collection->size = new_size;
}

/// @brief Checks if a page is empty
/// @param page the page to check if empty (must be fully initialized, e.g. using 'calloc()')
/// @return true if page contains only the default values
bool page_is_empty(page_t *page) {
    if (!page) {
        return true;
    }

    return memcmp(page, &empty_page, sizeof(empty_page)) == 0;
}

void page_collection_print(page_collection_t *collection, const char *name) {
    for (size_t i = 0; i < collection->size; i++) {
        printf("%s: PAGE %ld\n", name, i);
        printf("* id: %d\n", collection->pages[i]->id);
        printf("* prev_id: %d\n", collection->pages[i]->prev_id);
        printf("* next_id: %d\n", collection->pages[i]->next_id);
        printf("* unix_date: %lu\n", collection->pages[i]->unix_date);
        printf("* title: %s\n", collection->pages[i]->title);
        // TODO: Print parsed content
        printf("\n");
    }
}

void page_tokens_destroy(page_token_t *tokens) {
    if (!tokens) {
        return;
    }

    free(tokens);
}

void page_destroy(page_t *page) {
    page_tokens_destroy(page->tokens);
    free(page->title);
    free(page);
}

void page_collection_destroy(page_collection_t *collection) {
    for (size_t i = 0; i < collection->size; i++) {
        page_destroy(collection->pages[i]);
    }

    free(collection->pages);
    free(collection);
}
