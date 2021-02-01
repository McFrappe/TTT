#pragma once
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <curl/curl.h>

#include "shared.h"
#include "parser.h"
#include "errors.h"

typedef enum api_pages {
    TTT_PAGE_HOME = 100,
    TTT_PAGE_NEWS = 101,
    TTT_PAGE_FOREIGN_NEWS = 104,
    TTT_PAGE_ECONOMY = 200,
    TTT_PAGE_SPORT = 300,
    TTT_PAGE_STOCK_MARKET = 330,
    TTT_PAGE_SPORT_SERVICE = 376,
    TTT_PAGE_TV = 600,
    TTT_PAGE_CONTENTS = 700
} api_pages_t;

void api_initialize();
page_collection_t *api_get_page(uint16_t page);
page_collection_t *api_get_page_range(uint16_t range_start, uint16_t range_end);
void api_destroy();
