#include "api.h"
#include <stdlib.h>
#include <assert.h>
#include <curl/curl.h>

#define API_ID "terminaltexttv"
#define URL_BUF_SIZE 256
#define RANGE_BUF_SIZE 16

static CURL *curl = NULL;
static CURLcode res = -1;
static char url_buf[URL_BUF_SIZE];

static void create_page_range(char *buf, size_t buf_size, uint16_t start, uint16_t end) {
  assert(buf != NULL);
  assert(buf_size != 0);
  
  if (end == 0) {
    snprintf(buf, buf_size, "%d", start);
  } else {
    snprintf(buf, buf_size, "%d-%d", start, end);
  }
}

/// @brief Creates the endpoint URL for a specific page or range
/// @param start the start of the range (may not be 0)
/// @param end the end of the range (0 means no range)
/// @return the endpoint URL for the page between start and end or only start if end is 0
static void create_endpoint_url(char *buf, size_t buf_size, uint16_t start, uint16_t end) {
  assert(start != 0);
  
  char range[RANGE_BUF_SIZE];
  create_page_range(range, RANGE_BUF_SIZE, start, end);
  
  snprintf(
    buf,
    buf_size,
    "http://api.texttv.nu/api/get/%s?app=%s",
    range,
    API_ID
  );
}

static void make_request(uint16_t start, uint16_t end) {
  assert(start != 0);

  if (curl == NULL)
    api_intialize();

  if (!curl)
    return;

  create_endpoint_url(url_buf, URL_BUF_SIZE, start, end);
  
  curl_easy_setopt(
    curl,
    CURLOPT_URL,
    url_buf
  );
  
  // TODO: Where we last ended up.
  res = curl_easy_perform(curl);
  
  if (res != CURLE_OK) {
    printf("Fetch failed: %s", curl_easy_strerror(res));
    return;
  }
  
  // TODO: How do we get the response data? 

  curl_easy_cleanup(curl);
}

void api_intialize() {
  curl_global_init(CURL_GLOBAL_DEFAULT);
  curl = curl_easy_init();

  if (!curl) {
    printf("Failed to initialize curl");
    exit(1);
  }
}

void api_get_page(uint16_t page) {
  make_request(page, 0);
}

void api_get_page_range(uint16_t start, uint16_t end) {
  make_request(start, end);
}