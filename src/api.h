#pragma once
#include "parser.h"
#include <stdint.h>
#include <curl/curl.h>

enum pages {
  HOME = 100,
  NEWS = 101,
  FOREIGN_NEWS = 104,
  ECONOMY = 200,
  SPORT = 300,
  STOCK_MARKET = 330,
  SPORT_SERVICE = 376,
  TV = 600,
  CONTENTS = 700
};

typedef enum pages pages_t;

void api_intialize();
page_t *api_get_page(uint16_t page);
page_t *api_get_page_range(uint16_t range_start, uint16_t range_end);
void api_destroy();
