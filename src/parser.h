#pragma once
#include "pages.h"
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

page_collection_t *parser_convert_to_pages(const char *data, size_t size);
