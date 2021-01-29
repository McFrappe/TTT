#include "pages.h"

void page_collection_print(page_collection_t *collection, const char *name) {
    for (size_t i = 0; i < collection->size; i++) {
        printf("%s: PAGE %ld\n", name, i);
        printf("* id: %d\n", collection->pages[i]->id);
        printf("* prev_id: %d\n", collection->pages[i]->prev_id);
        printf("* next_id: %d\n", collection->pages[i]->next_id);
        printf("* unix_date: %zd\n", collection->pages[i]->unix_date);
        printf("* title: %s\n", collection->pages[i]->title);
        printf("* content_size: %zd\n", collection->pages[i]->content_size);
        // TODO: Print parsed content
        printf("\n");
    }
}

// Internal destroy function for content that is in a page_t struct
static void post_content_destroy(post_content_t **content, size_t content_size) {
    for (size_t i = 0; i < content_size; i++) {
        post_content_t *current = content[i];

        free(current->title);
        free(current->text);
        free(current);
    }

    free(content);
}

void page_destroy(page_t *page) {
    post_content_destroy(page->content, page->content_size);
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
