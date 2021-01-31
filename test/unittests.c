#include <stdio.h>
#include <stdbool.h>
#include <CUnit/Basic.h>
#include "../src/pages.h"
#include "../src/parser.h"

#define JSON_DATA_PAGE_PATH "./test/data/index.json"
#define JSON_DATA_PAGE_RANGE_PATH "./test/data/range.json"
#define JSON_DATA_PAGE_RANGE_LARGE_PATH "./test/data/range_large.json"

typedef struct json {
    char *data;
    size_t length;
} json_t;

static json_t JSON_DATA_PAGE;
static json_t JSON_DATA_PAGE_RANGE;
static json_t JSON_DATA_PAGE_RANGE_LARGE;

bool load_test_data(json_t *dest, const char *path) {
    FILE *f = fopen(path, "r");

    if (!f) {
        return false;
    }

    fseek (f, 0, SEEK_END);
    dest->length = ftell(f);
    fseek (f, 0, SEEK_SET);
    dest->data = calloc(dest->length + 1, sizeof(char));

    if (!dest->data || !fread(dest->data, sizeof(char), dest->length, f)) {
        return false;
    }

    dest->data[dest->length] = '\0';
    fclose(f);

    return true;
}\

void destroy_test_data(json_t *json) {
    free(json->data);
}

void assert_page_collection(page_collection_t *collection, size_t size) {
    CU_ASSERT_EQUAL(collection->size, size);

    if (collection->size == 0) {
        CU_ASSERT_PTR_NULL_FATAL(collection->pages);
    } else {
        CU_ASSERT_PTR_NOT_NULL_FATAL(collection->pages);
    }
}

void assert_numeric_value(size_t actual, size_t expected) {
    CU_ASSERT_EQUAL(actual, expected);

    if (actual != expected) {
        printf("Expected: '%zd', but got: '%zd'\n", expected, actual);
    }
}

void assert_string_value(const char *actual, const char *expected) {
    int diff = strcmp(actual, expected);
    CU_ASSERT_EQUAL(diff, 0);

    if (diff != 0) {
        printf("Expected: '%s', but got: '%s'\n", expected, actual);
    }
}

void assert_parsed_page(
    page_t *page,
    uint16_t id,
    uint16_t prev_id,
    uint16_t next_id,
    uint64_t unix_date,
    const char *title,
    page_content_t *content
) {
    CU_ASSERT_PTR_NOT_NULL_FATAL(page);

    if (!page) {
        return;
    }

    assert_numeric_value(page->id, id);
    assert_numeric_value(page->prev_id, prev_id);
    assert_numeric_value(page->next_id, next_id);
    assert_numeric_value(page->unix_date, unix_date);

    if (!title || !page->title) {
        CU_ASSERT_PTR_EQUAL(page->title, title);
    } else {
        assert_string_value(page->title, title);
    }

    if (!content) {
        CU_ASSERT_PTR_NULL(page->content);
    } else {
        CU_ASSERT_PTR_NOT_NULL(page->content);
        // TODO: Compare content
    }
}

void test_page_null_string() {
    CU_ASSERT_PTR_NULL(parser_get_page_collection(NULL, 0));
}

void test_page_empty_string() {
    CU_ASSERT_PTR_NULL(parser_get_page_collection("", 0));
}

void test_page_empty_array() {
    CU_ASSERT_PTR_NULL(parser_get_page_collection("[]", 2));
}

void test_page_object_without_array() {
    CU_ASSERT_PTR_NULL(parser_get_page_collection("{}", 2));
}

void test_page_empty_title() {
    char *str = "[{\"title\": \"\"}]";
    page_collection_t *collection = parser_get_page_collection(str, strlen(str));

    assert_page_collection(collection, 1);

    assert_parsed_page(
        collection->pages[0],
        -1,
        -1,
        -1,
        -1,
        NULL,
        NULL
    );

    page_collection_destroy(collection);
}

void test_page_single_char_title() {
    char *str = "[{\"title\": \"x\"}]";
    page_collection_t *collection = parser_get_page_collection(str, strlen(str));

    assert_page_collection(collection, 1);

    assert_parsed_page(
        collection->pages[0],
        -1,
        -1,
        -1,
        -1,
        "x",
        NULL
    );

    page_collection_destroy(collection);
}

void test_page_title_unescaped() {
    char *str = "[{\"title\": \"abc-\\u00e4\\u00e5\\u00c4\\u00c5\\u00f6\\u00d6-def\"}]";
    page_collection_t *collection = parser_get_page_collection(str, strlen(str));

    assert_page_collection(collection, 1);

    assert_parsed_page(
        collection->pages[0],
        -1,
        -1,
        -1,
        -1,
        "abc-aaAAoO-def",
        NULL
    );

    page_collection_destroy(collection);
}

void test_page_invalid_keys() {
    char *str = "[{\"invalid\": \"asd\", \"xxx\": 100, \"yyy\": [\"zzz\"]}]";
    page_collection_t *collection = parser_get_page_collection(str, strlen(str));

    assert_page_collection(collection, 1);

    assert_parsed_page(
        collection->pages[0],
        -1,
        -1,
        -1,
        -1,
        NULL,
        NULL
    );

    page_collection_destroy(collection);
}

void test_page_large_content_array() {
    char *str = "[{\"content\": [\"xxx\", \"yyy\", \"zzz\"]}]";
    page_collection_t *collection = parser_get_page_collection(str, strlen(str));

    assert_page_collection(collection, 1);
    assert_parsed_page(
        collection->pages[0],
        -1,
        -1,
        -1,
        -1,
        NULL,
        NULL
    );

    page_collection_destroy(collection);
}

void test_page_invalid() {
    // Contains the response of an invalid page, e.g. page id 0
    char *str = "[{\"num\":null,\"title\":null,\"content\":[],\"next_page\":null,\
                 \"prev_page\":null,\"date_updated_unix\":null,\
                 \"permalink\":\"https://texttv.nu/0/-0\",\"id\":null}]";
    page_collection_t *collection = parser_get_page_collection(str, strlen(str));

    assert_page_collection(collection, 1);
    assert_parsed_page(
        collection->pages[0],
        -1,
        -1,
        -1,
        -1,
        NULL,
        NULL
    );

    page_collection_destroy(collection);
}

void test_page_collection_resize() {
    // The parser will read this as 6 possible pages, however 3 are strings
    // and therefore will not be parsed as a page. This means that we should
    // get a page collection of only 3 pages when parsed.
    char *str = "[{}, {}, {}, \"xxx\", \"yyy\", \"zzz\"]";
    page_collection_t *collection = parser_get_page_collection(str, strlen(str));

    assert_page_collection(collection, 3);
    assert_parsed_page(
        collection->pages[0],
        -1,
        -1,
        -1,
        -1,
        NULL,
        NULL
    );

    assert_parsed_page(
        collection->pages[1],
        -1,
        -1,
        -1,
        -1,
        NULL,
        NULL
    );

    assert_parsed_page(
        collection->pages[2],
        -1,
        -1,
        -1,
        -1,
        NULL,
        NULL
    );

    page_collection_destroy(collection);
}

// Validates the parsing of the file at JSON_DATA_PAGE_PATH
void test_page_single() {
    page_collection_t *collection = parser_get_page_collection(
        JSON_DATA_PAGE.data, JSON_DATA_PAGE.length
    );

    assert_page_collection(collection, 1);
    assert_parsed_page(
        collection->pages[0],
        200,
        199,
        201,
        1612004371,
        "Svt",
        NULL
    );

    page_collection_destroy(collection);
}

// Validates the parsing of the file at JSON_DATA_PAGE_RANGE_PATH
void test_page_range() {
    page_collection_t *collection = parser_get_page_collection(
        JSON_DATA_PAGE_RANGE.data, JSON_DATA_PAGE_RANGE.length
    );

    assert_page_collection(collection, 5);

    assert_parsed_page(
        collection->pages[0],
        100,
        100,
        101,
        1612004855,
        "USA infor munskyddskrav for resande | Svensk test av virusmutationer drojer | Inrikes",
        NULL
    );

    assert_parsed_page(
        collection->pages[1],
        101,
        100,
        102,
        1612004794,
        "SVT Text",
        NULL
    );

    assert_parsed_page(
        collection->pages[2],
        102,
        101,
        103,
        1612007994,
        "SVT Text",
        NULL
    );

    assert_parsed_page(
        collection->pages[3],
        103,
        102,
        104,
        1612007994,
        "SVT Text",
        NULL
    );

    assert_parsed_page(
        collection->pages[4],
        104,
        103,
        105,
        1612004496,
        "SVT Text",
        NULL
    );

    page_collection_destroy(collection);
}

// Validates the parsing of the file at JSON_DATA_PAGE_RANGE_PATH
void test_page_range_large() {
    page_collection_t *collection = parser_get_page_collection(
        JSON_DATA_PAGE_RANGE_LARGE.data, JSON_DATA_PAGE_RANGE_LARGE.length
    );

    assert_page_collection(collection, 6);

    assert_parsed_page(
        collection->pages[0],
        200,
        199,
        201,
        1612004371,
        "Svt",
        NULL
    );

    assert_parsed_page(
        collection->pages[1],
        201,
        200,
        202,
        1612028945,
        "Svt",
        NULL
    );

    assert_parsed_page(
        collection->pages[2],
        202,
        201,
        203,
        1612028945,
        "Kalla:",
        NULL
    );

    assert_parsed_page(
        collection->pages[3],
        203,
        202,
        204,
        1612028945,
        "Kalla:",
        NULL
    );

    assert_parsed_page(
        collection->pages[4],
        204,
        203,
        205,
        1612028945,
        "Kalla:",
        NULL
    );

    assert_parsed_page(
        collection->pages[5],
        205,
        204,
        206,
        1612028945,
        "Kalla:",
        NULL
    );

    page_collection_destroy(collection);
}

int main() {
    if (
        !load_test_data(&JSON_DATA_PAGE, JSON_DATA_PAGE_PATH) ||
        !load_test_data(&JSON_DATA_PAGE_RANGE, JSON_DATA_PAGE_RANGE_PATH) ||
        !load_test_data(&JSON_DATA_PAGE_RANGE_LARGE, JSON_DATA_PAGE_RANGE_LARGE_PATH)
    ) {
        printf("Failed to load test JSON data!\n");
        exit(1);
    }

    CU_initialize_registry();
    CU_pSuite page_parser_suite = CU_add_suite("Page parser tests", 0, 0);
    CU_add_test(page_parser_suite, "test_page_null_string", test_page_null_string);
    CU_add_test(page_parser_suite, "test_page_empty_string", test_page_empty_string);
    CU_add_test(page_parser_suite, "test_page_empty_array", test_page_empty_array);
    CU_add_test(page_parser_suite, "test_page_empty_title", test_page_empty_title);
    CU_add_test(page_parser_suite, "test_page_single_char_title", test_page_single_char_title);
    CU_add_test(page_parser_suite, "test_page_invalid_keys", test_page_invalid_keys);
    CU_add_test(page_parser_suite, "test_page_object_without_array", test_page_object_without_array);
    CU_add_test(page_parser_suite, "test_page_title_unescaped", test_page_title_unescaped);
    CU_add_test(page_parser_suite, "test_page_invalid", test_page_invalid);
    CU_add_test(page_parser_suite, "test_page_collection_resize", test_page_collection_resize);
    CU_add_test(page_parser_suite, "test_page_large_content_array", test_page_large_content_array);
    CU_add_test(page_parser_suite, "test_page_single", test_page_single);
    CU_add_test(page_parser_suite, "test_page_range", test_page_range);
    CU_add_test(page_parser_suite, "test_page_range_large", test_page_range_large);
    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    CU_cleanup_registry();
    destroy_test_data(&JSON_DATA_PAGE);
    destroy_test_data(&JSON_DATA_PAGE_RANGE);
    destroy_test_data(&JSON_DATA_PAGE_RANGE_LARGE);
    return 0;
}
