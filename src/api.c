#include "api.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <curl/curl.h>

#define API_ID "terminaltexttv"
#define URL_BUF_SIZE 256
#define RANGE_BUF_SIZE 16

typedef struct response_chunk {
    char *data;
    size_t size;
} response_chunk_t;

static CURL *curl = NULL;
static CURLcode res_code = -1;
static char url_buf[URL_BUF_SIZE];

static size_t write_callback(void *data, size_t size, size_t nmemb, void *extra) {
    size_t realsize = size * nmemb;
    response_chunk_t *mem = extra;
    char *ptr = realloc(mem->data, mem->size + realsize + 1);

    if (ptr == NULL) {
        printf("Out of memory!\n");
        return 0;
    }

    mem->data = ptr;
    memcpy(&(mem->data[mem->size]), data, realsize);
    mem->size += realsize;
    mem->data[mem->size] = 0;
    return realsize;
}

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


// TODO: Return error(s) and display in UI
static page_collection_t *make_request(uint16_t start, uint16_t end) {
    assert(start != 0);

    if (!curl)
        api_intialize();

    create_endpoint_url(url_buf, URL_BUF_SIZE, start, end);
    // Create chunk to store data in
    response_chunk_t chunk = {
        // TODO: Allocate more memory at once to prevent unnecessary realloc's?
        .data = malloc(1),
        .size = 0
    };
    curl_easy_setopt(curl, CURLOPT_URL,           url_buf);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS,    1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA,     &chunk);
    curl_easy_setopt(curl, CURLOPT_USERAGENT,     "libcurl-agent/1.0");
    res_code = curl_easy_perform(curl);

    if (res_code != CURLE_OK) {
        if (chunk.data)
            free(chunk.data);

        printf("Fetch failed: %s\n", curl_easy_strerror(res_code));
        return NULL;
    }

    //printf("Response body: %s\n", chunk.data);
    page_collection_t *collection = parser_convert_to_pages(chunk.data, chunk.size);
    free(chunk.data);

    if (!collection) {
        printf("Could not parse response body!\n");
        return NULL;
    }

    return collection;
}

void api_intialize() {
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (!curl) {
        printf("Failed to initialize curl");
        exit(1);
    }
}

page_collection_t *api_get_page(uint16_t page_id) {
    return make_request(page_id, 0);
}

page_collection_t *api_get_page_range(uint16_t start, uint16_t end) {
    return make_request(start, end);
}

void api_destroy() {
    // Valgrind detects memory that does not get free'd.
    // This seems to be a known issue (?)
    // https://stackoverflow.com/questions/11494950/memory-leak-from-curl-library
    // The errors in 'memtest' are hidden using a valrind suppression file.
    curl_easy_cleanup(curl);
    curl_global_cleanup();
}
