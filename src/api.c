#include "api.h"
#include <stdlib.h>
#include <assert.h>
#include <curl/curl.h>

#define APP_ID terminaltexttv

static CURL *curl = NULL;

static char *create_endpoint_url(char *buffer, uint16_t page) {
  // TODO: Append page id
  return buffer;
}

void api_intialize() {
  curl_global_init(CURL_GLOBAL_DEFAULT);
  curl = curl_easy_init();

  if (!curl) exit(1);
}

void api_get_page(uint16_t page) {
  assert(page != 0);

  if (curl == NULL) {
    api_intialize();
  }

  if (curl)
    curl_easy_setopt(curl, CURLOPT_URL, "https://example.com/");
}
