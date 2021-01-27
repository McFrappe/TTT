#include "parser.h"
#include <stdlib.h>

struct post_content {
  char *title;
  char *text;
  uint16_t id;
  post_content_type_t type;
  post_content_t *next;
};

struct page {
  uint16_t id, prev_id, next_id;
  uint64_t unix_date;
  char *title;
  post_content_t *content;
};

page_t *parser_convert_to_page(char *data) {
  page_t *page = calloc(1, sizeof(page_t));
  *page = (page_t) {
    .id = 0,
    .prev_id = 0,
    .next_id = 0,
    .unix_date = 0,
    .title = "Hello from parser",
    .content = NULL
  };

  return page;
}
