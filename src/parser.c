#include "parser.h"
#include "../lib/jsmn.h"

#define NO_TOKENS 128

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

page_t *parser_convert_to_page(const char *data, size_t size) {
  if (!data || size == 0) 
    return NULL;

  jsmn_parser parser;
  jsmn_init(&parser);
  jsmntok_t tokens[NO_TOKENS];
  
  int keys = jsmn_parse(&parser, data, size, tokens, sizeof(tokens) / sizeof(tokens[0]));
  printf("keys: %d\n", keys);
  
  if (keys < 0) {
    printf("Failed to parse response body!\n");
    return NULL;
  }

  if (keys < 1 || tokens[0].type != JSMN_ARRAY) {
    printf("Parser expected array at top level!\n");
    return NULL;
  }
  
  int pages = 0;
  
  for (int i = 1; i < keys; i++) {
    if (tokens[i].type == JSMN_OBJECT) {
      printf("Contains page!\n");
      pages++; 
    }
  }

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