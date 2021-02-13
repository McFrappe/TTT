#pragma once
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <limits.h>

#include "pages.h"
#include "shared.h"
#include "errors.h"

page_token_t *html_parser_get_page_tokens(const char *data, size_t size);