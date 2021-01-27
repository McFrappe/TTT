#pragma once
#include <stdint.h>

enum post_content_type { HEADLINE, ARTICLE };

typedef struct page page_t;
typedef struct post post_t;
typedef struct post_content post_content_t;
typedef enum post_content_type post_content_type_t;

page_t *parser_convert_to_page(char *data);
