#pragma once
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <limits.h>

#include "pages.h"

page_t *parser_get_page_content(const char *content, size_t size);
page_collection_t *parser_get_page_collection(const char *data, size_t size);
