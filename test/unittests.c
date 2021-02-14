#include <stdio.h>
#include <stdbool.h>
#include <CUnit/Basic.h>
#include "../src/pages.h"
#include "../src/parser.h"
#include "../src/html_parser.h"

#define JSON_DATA_PAGE_PATH "./test/data/index.json"
#define JSON_DATA_PAGE_RANGE_PATH "./test/data/range.json"
#define JSON_DATA_PAGE_RANGE_LARGE_PATH "./test/data/range_large.json"
#define HTML_DATA_PAGE_1_PATH "./test/data/page1.html"

#define NO_HREF 0

typedef struct file_data {
    char *data;
    size_t length;
} file_data_t;

static file_data_t JSON_DATA_PAGE;
static file_data_t JSON_DATA_PAGE_RANGE;
static file_data_t JSON_DATA_PAGE_RANGE_LARGE;
static file_data_t HTML_DATA_PAGE_1;

bool load_test_data(file_data_t *dest, const char *path) {
    FILE *f = fopen(path, "r");

    if (!f) {
        return false;
    }

    fseek(f, 0, SEEK_END);
    dest->length = ftell(f);
    fseek(f, 0, SEEK_SET);
    dest->data = calloc(dest->length + 1, sizeof(char));

    if (!dest->data || !fread(dest->data, sizeof(char), dest->length, f)) {
        return false;
    }

    dest->data[dest->length] = '\0';
    fclose(f);

    return true;
}

void destroy_test_data(file_data_t *json) {
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
    bool has_tokens
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

    if (!has_tokens) {
        CU_ASSERT_PTR_NULL(page->tokens);
    } else {
        CU_ASSERT_PTR_NOT_NULL(page->tokens);
    }
}

void assert_parsed_page_tokens(page_t *page) {
    page_token_t *value = page->tokens;

    if (error_is_set()) {
        printf(" %s - ", error_get_string());
    }

    if (!value) {
        page_destroy(page);
        error_reset();
    }

    // For verbosity
    CU_ASSERT_FALSE(error_is_set());
    CU_ASSERT_PTR_NOT_NULL_FATAL(value);
}

void assert_token(
    page_token_t **token,
    const char *expected_text,
    uint16_t expected_href,
    page_token_type_t expected_type,
    page_token_attr_t expected_bg,
    page_token_attr_t expected_fg,
    page_token_attr_t expected_extra
) {
    page_token_t *current = (*token);
    CU_ASSERT_PTR_NOT_NULL_FATAL(current);

    if (
        expected_text && (
        expected_type == PAGE_TOKEN_TEXT ||
        expected_type == PAGE_TOKEN_HEADER ||
        expected_type == PAGE_TOKEN_LINK)
    ) {
        CU_ASSERT_PTR_NOT_NULL(current->text);

        if (current->text) {
            CU_ASSERT_STRING_EQUAL(expected_text, current->text);
            CU_ASSERT_EQUAL(strlen(expected_text), current->length);
        }
    } else {
        CU_ASSERT_PTR_NULL(current->text);
    }

    CU_ASSERT_EQUAL(expected_href, current->href);
    CU_ASSERT_EQUAL(expected_type, current->type);
    CU_ASSERT_EQUAL(expected_bg, current->style.bg);
    CU_ASSERT_EQUAL(expected_fg, current->style.fg);
    CU_ASSERT_EQUAL(expected_extra, current->style.extra);

    // Move forward in tokens array
    (*token) = (*token)->next;
}

void assert_token_end(page_token_t *cursor) {
    // Make sure that there are no more tokens
    CU_ASSERT_PTR_NULL(cursor);
}

void test_page_null_string() {
    CU_ASSERT_PTR_NULL(parser_get_page_collection(NULL, 0));
    CU_ASSERT_TRUE(error_is_set());
    error_reset();
}

void test_page_empty_string() {
    CU_ASSERT_PTR_NULL(parser_get_page_collection("", 0));
    CU_ASSERT_TRUE(error_is_set());
    error_reset();
}

void test_page_empty_array() {
    CU_ASSERT_PTR_NULL(parser_get_page_collection("[]", 2));
    CU_ASSERT_TRUE(error_is_set());
    error_reset();
}

void test_page_object_without_array() {
    CU_ASSERT_PTR_NULL(parser_get_page_collection("{}", 2));
    CU_ASSERT_TRUE(error_is_set());
    error_reset();
}

void test_page_empty_title() {
    char *str = "[{\"title\": \"\"}]";
    page_collection_t *collection = parser_get_page_collection(str, strlen(str));
    CU_ASSERT_FALSE(error_is_set());

    assert_page_collection(collection, 1);

    assert_parsed_page(
        collection->pages[0],
        -1,
        -1,
        -1,
        -1,
        NULL,
        false
    );

    page_collection_destroy(collection);
    error_reset();
}

void test_page_single_char_title() {
    char *str = "[{\"title\": \"x\"}]";
    page_collection_t *collection = parser_get_page_collection(str, strlen(str));
    CU_ASSERT_FALSE(error_is_set());

    assert_page_collection(collection, 1);

    assert_parsed_page(
        collection->pages[0],
        -1,
        -1,
        -1,
        -1,
        "x",
        false
    );

    page_collection_destroy(collection);
    error_reset();
}

void test_page_invalid_keys() {
    char *str = "[{\"invalid\": \"asd\", \"xxx\": 100, \"yyy\": [\"zzz\"]}]";
    page_collection_t *collection = parser_get_page_collection(str, strlen(str));
    CU_ASSERT_FALSE(error_is_set());

    assert_page_collection(collection, 1);

    assert_parsed_page(
        collection->pages[0],
        -1,
        -1,
        -1,
        -1,
        NULL,
        false
    );

    page_collection_destroy(collection);
    error_reset();
}

void test_page_large_content_array() {
    char *str = "[{\"content\": [\"xxx\", \"yyy\", \"zzz\"]}]";
    page_collection_t *collection = parser_get_page_collection(str, strlen(str));

    // The content strings are invalid html and won't be parsed correctly
    CU_ASSERT_TRUE(error_is_set());

    assert_page_collection(collection, 1);
    assert_parsed_page(
        collection->pages[0],
        -1,
        -1,
        -1,
        -1,
        NULL,
        false
    );

    page_collection_destroy(collection);
    error_reset();
}

void test_page_invalid() {
    // Contains the response of an invalid page, e.g. page id 0
    char *str = "[{\"num\":null,\"title\":null,\"content\":[],\"next_page\":null,\
                 \"prev_page\":null,\"date_updated_unix\":null,\
                 \"permalink\":\"https://texttv.nu/0/-0\",\"id\":null}]";
    page_collection_t *collection = parser_get_page_collection(str, strlen(str));
    CU_ASSERT_TRUE(error_is_set());

    assert_page_collection(collection, 1);
    assert_parsed_page(
        collection->pages[0],
        -1,
        -1,
        -1,
        -1,
        NULL,
        false
    );

    page_collection_destroy(collection);
    error_reset();
}

void test_page_collection_empty_objects() {
    char *str = "[{}, {}, {}, \"xxx\", \"yyy\", \"zzz\"]";
    page_collection_t *collection = parser_get_page_collection(str, strlen(str));
    CU_ASSERT_TRUE(error_is_set());
    CU_ASSERT_PTR_NULL(collection);
    error_reset();
}

// Validates the parsing of the file at JSON_DATA_PAGE_PATH
void test_page_single() {
    page_collection_t *collection = parser_get_page_collection(
        JSON_DATA_PAGE.data, JSON_DATA_PAGE.length
    );

    CU_ASSERT_FALSE(error_is_set());

    assert_page_collection(collection, 1);
    assert_parsed_page(
        collection->pages[0],
        200,
        199,
        201,
        1612004371,
        "Svt",
        true
    );

    page_collection_destroy(collection);
    error_reset();
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
        "USA inf\\u00f6r munskyddskrav f\\u00f6r resande | Svensk test av virusmutationer dr\\u00f6jer | Inrikes",
        true
    );

    assert_parsed_page(
        collection->pages[1],
        101,
        100,
        102,
        1612004794,
        "SVT Text",
        true
    );

    assert_parsed_page(
        collection->pages[2],
        102,
        101,
        103,
        1612007994,
        "SVT Text",
        true
    );

    assert_parsed_page(
        collection->pages[3],
        103,
        102,
        104,
        1612007994,
        "SVT Text",
        true
    );

    assert_parsed_page(
        collection->pages[4],
        104,
        103,
        105,
        1612004496,
        "SVT Text",
        true
    );

    page_collection_destroy(collection);
    error_reset();
}

// Validates the parsing of the file at JSON_DATA_PAGE_RANGE_PATH
void test_page_range_large() {
    page_collection_t *collection = parser_get_page_collection(
        JSON_DATA_PAGE_RANGE_LARGE.data, JSON_DATA_PAGE_RANGE_LARGE.length
    );

    CU_ASSERT_FALSE(error_is_set());

    assert_page_collection(collection, 6);

    assert_parsed_page(
        collection->pages[0],
        200,
        199,
        201,
        1612004371,
        "Svt",
        true
    );

    assert_parsed_page(
        collection->pages[1],
        201,
        200,
        202,
        1612028945,
        "Svt",
        true
    );

    assert_parsed_page(
        collection->pages[2],
        202,
        201,
        203,
        1612028945,
        "K\\u00e4lla:",
        true
    );

    assert_parsed_page(
        collection->pages[3],
        203,
        202,
        204,
        1612028945,
        "K\\u00e4lla:",
        true
    );

    assert_parsed_page(
        collection->pages[4],
        204,
        203,
        205,
        1612028945,
        "K\\u00e4lla:",
        true
    );

    assert_parsed_page(
        collection->pages[5],
        205,
        204,
        206,
        1612028945,
        "K\\u00e4lla:",
        true
    );

    page_collection_destroy(collection);
    error_reset();
}

void test_page_html_null() {
    page_t *page = page_create_empty();
    html_parser_get_page_tokens(page, "", 0);

    CU_ASSERT_TRUE(error_is_set());
    CU_ASSERT_PTR_NULL(page->tokens);

    page_destroy(page);
    error_reset();
}

void test_page_html_invalid_start_tag() {
    const char *str = "<a href=\"/100\">100</a>";
    page_t *page = page_create_empty();
    html_parser_get_page_tokens(page, str, strlen(str));

    CU_ASSERT_TRUE(error_is_set());
    CU_ASSERT_PTR_NULL(page->tokens);

    page_destroy(page);
    error_reset();
}

void test_page_html_unexpected_end() {
    const char *str = "<span class=\"bgB B\">100";
    page_t *page = page_create_empty();
    html_parser_get_page_tokens(page, str, strlen(str));

    CU_ASSERT_TRUE(error_is_set());
    CU_ASSERT_PTR_NULL(page->tokens);

    page_destroy(page);
    error_reset();
}

void test_page_html_empty_class() {
    const char *str = "<span class=\"\">100</span>";
    page_t *page = page_create_empty();
    html_parser_get_page_tokens(page, str, strlen(str));

    assert_parsed_page_tokens(page);

    page_token_t *cursor = page->tokens;

    assert_token(&cursor,
        "100",
        NO_HREF,
        PAGE_TOKEN_TEXT,
        PAGE_TOKEN_ATTR_BG_BLACK,
        PAGE_TOKEN_ATTR_WHITE,
        PAGE_TOKEN_ATTR_NONE
    );

    assert_token_end(cursor);

    page_destroy(page);
    error_reset();
}

void test_page_html_nested_a_tag() {
    const char *str = "<span class=\"bgB B\"><a href=\"/100\">i am link</a></span>";
    page_t *page = page_create_empty();
    html_parser_get_page_tokens(page, str, strlen(str));

    assert_parsed_page_tokens(page);

    page_token_t *cursor = page->tokens;

    assert_token(&cursor,
        NULL,
        NO_HREF,
        PAGE_TOKEN_TEXT,
        PAGE_TOKEN_ATTR_BG_BLUE,
        PAGE_TOKEN_ATTR_BLUE,
        PAGE_TOKEN_ATTR_NONE
    );

    assert_token(&cursor,
        "i am link",
        100,
        PAGE_TOKEN_LINK,
        PAGE_TOKEN_ATTR_BG_BLUE,
        PAGE_TOKEN_ATTR_BLUE,
        PAGE_TOKEN_ATTR_NONE
    );

    assert_token_end(cursor);

    page_destroy(page);
    error_reset();
}

void test_page_html_h1_tag() {
    const char *str = "<h1 class=\"bgY B\">some title</h1>";
    page_t *page = page_create_empty();
    html_parser_get_page_tokens(page, str, strlen(str));

    assert_parsed_page_tokens(page);

    page_token_t *cursor = page->tokens;

    assert_token(&cursor,
        "some title",
        NO_HREF,
        PAGE_TOKEN_TEXT,
        PAGE_TOKEN_ATTR_BG_YELLOW,
        PAGE_TOKEN_ATTR_BLUE,
        PAGE_TOKEN_ATTR_NONE
    );

    assert_token_end(cursor);

    page_destroy(page);
    error_reset();
}

void test_page_html_h1_tag_empty_class() {
    const char *str = "<h1 class=\"\">some title</h1>";
    page_t *page = page_create_empty();
    html_parser_get_page_tokens(page, str, strlen(str));

    assert_parsed_page_tokens(page);

    page_token_t *cursor = page->tokens;

    assert_token(&cursor,
        "some title",
        NO_HREF,
        PAGE_TOKEN_TEXT,
        PAGE_TOKEN_ATTR_BG_BLACK,
        PAGE_TOKEN_ATTR_WHITE,
        PAGE_TOKEN_ATTR_NONE
    );

    assert_token_end(cursor);

    page_destroy(page);
    error_reset();
}

void test_page_html_h1_tag_nested_a_tag() {
    const char *str = "<h1 class=\"bgB B\"><a href=\"/100\">i am link</a></h1>";
    page_t *page = page_create_empty();
    html_parser_get_page_tokens(page, str, strlen(str));

    assert_parsed_page_tokens(page);

    page_token_t *cursor = page->tokens;

    assert_token(&cursor,
        NULL,
        NO_HREF,
        PAGE_TOKEN_TEXT,
        PAGE_TOKEN_ATTR_BG_BLUE,
        PAGE_TOKEN_ATTR_BLUE,
        PAGE_TOKEN_ATTR_NONE
    );

    assert_token(&cursor,
        "i am link",
        100,
        PAGE_TOKEN_LINK,
        PAGE_TOKEN_ATTR_BG_BLUE,
        PAGE_TOKEN_ATTR_BLUE,
        PAGE_TOKEN_ATTR_NONE
    );

    assert_token_end(cursor);

    page_destroy(page);
    error_reset();
}

void test_page_html_bold_text() {
    const char *str = "<span class=\"bgB B DH\">hello</span>";
    page_t *page = page_create_empty();
    html_parser_get_page_tokens(page, str, strlen(str));

    assert_parsed_page_tokens(page);

    page_token_t *cursor = page->tokens;

    assert_token(&cursor,
        "hello",
        NO_HREF,
        PAGE_TOKEN_TEXT,
        PAGE_TOKEN_ATTR_BG_BLUE,
        PAGE_TOKEN_ATTR_BLUE,
        PAGE_TOKEN_ATTR_BOLD
    );

    assert_token_end(cursor);

    page_destroy(page);
    error_reset();
}

void test_page_html_newline_no_whitespace() {
    const char *str = "<span class=\"bgB B\">hello\\n</span>";
    page_t *page = page_create_empty();
    html_parser_get_page_tokens(page, str, strlen(str));

    assert_parsed_page_tokens(page);

    page_token_t *cursor = page->tokens;

    assert_token(&cursor,
        "hello",
        NO_HREF,
        PAGE_TOKEN_TEXT,
        PAGE_TOKEN_ATTR_BG_BLUE,
        PAGE_TOKEN_ATTR_BLUE,
        PAGE_TOKEN_ATTR_NONE
    );

    assert_token_end(cursor);

    page_destroy(page);
    error_reset();
}

void test_page_html_newline_whitespace() {
    const char *str = "<span class=\"bgB B\">hello\\n \\n </span>";
    page_t *page = page_create_empty();
    html_parser_get_page_tokens(page, str, strlen(str));

    assert_parsed_page_tokens(page);

    page_token_t *cursor = page->tokens;

    assert_token(&cursor,
        "hello",
        NO_HREF,
        PAGE_TOKEN_TEXT,
        PAGE_TOKEN_ATTR_BG_BLUE,
        PAGE_TOKEN_ATTR_BLUE,
        PAGE_TOKEN_ATTR_NONE
    );

    assert_token(&cursor,
        " ",
        NO_HREF,
        PAGE_TOKEN_TEXT,
        PAGE_TOKEN_ATTR_BG_BLUE,
        PAGE_TOKEN_ATTR_BLUE,
        PAGE_TOKEN_ATTR_NONE
    );

    assert_token(&cursor,
        " ",
        NO_HREF,
        PAGE_TOKEN_TEXT,
        PAGE_TOKEN_ATTR_BG_BLUE,
        PAGE_TOKEN_ATTR_BLUE,
        PAGE_TOKEN_ATTR_NONE
    );

    assert_token_end(cursor);

    page_destroy(page);
    error_reset();
}

void test_page_html_span_separator() {
    const char *str = "<span class=\"bgB B\">hello</span>\\n <span class=\"bgB B\">hello2</span>";
    page_t *page = page_create_empty();
    html_parser_get_page_tokens(page, str, strlen(str));

    assert_parsed_page_tokens(page);

    page_token_t *cursor = page->tokens;

    assert_token(&cursor,
        "hello",
        NO_HREF,
        PAGE_TOKEN_TEXT,
        PAGE_TOKEN_ATTR_BG_BLUE,
        PAGE_TOKEN_ATTR_BLUE,
        PAGE_TOKEN_ATTR_NONE
    );

    // Assert that the whitespace separator uses the default styling
    assert_token(&cursor,
        " ",
        NO_HREF,
        PAGE_TOKEN_TEXT,
        PAGE_TOKEN_ATTR_BG_BLACK,
        PAGE_TOKEN_ATTR_WHITE,
        PAGE_TOKEN_ATTR_NONE
    );

    assert_token(&cursor,
        "hello2",
        NO_HREF,
        PAGE_TOKEN_TEXT,
        PAGE_TOKEN_ATTR_BG_BLUE,
        PAGE_TOKEN_ATTR_BLUE,
        PAGE_TOKEN_ATTR_NONE
    );

    assert_token_end(cursor);

    page_destroy(page);
    error_reset();
}

void test_page_html_1() {
    page_t *page = page_create_empty();
    html_parser_get_page_tokens(page, HTML_DATA_PAGE_1.data, HTML_DATA_PAGE_1.length);
    page_token_t *cursor = page->tokens;

    assert_parsed_page_tokens(page);

    assert_token(&cursor,
        " 700 SVT Text        Torsdag 28 jan 2021",
        NO_HREF,
        PAGE_TOKEN_HEADER,
        PAGE_TOKEN_ATTR_BG_BLACK,
        PAGE_TOKEN_ATTR_WHITE,
        PAGE_TOKEN_ATTR_NONE
    );

    assert_token(
        &cursor,
        " ",
        NO_HREF,
        PAGE_TOKEN_TEXT,
        PAGE_TOKEN_ATTR_BG_BLACK,
        PAGE_TOKEN_ATTR_WHITE,
        PAGE_TOKEN_ATTR_NONE
    );

    assert_token(
        &cursor,
        " ",
        NO_HREF,
        PAGE_TOKEN_TEXT,
        PAGE_TOKEN_ATTR_BG_YELLOW,
        PAGE_TOKEN_ATTR_YELLOW,
        PAGE_TOKEN_ATTR_NONE
    );

    assert_token(
        &cursor,
        " ",
        NO_HREF,
        PAGE_TOKEN_TEXT,
        PAGE_TOKEN_ATTR_BG_YELLOW,
        PAGE_TOKEN_ATTR_YELLOW,
        PAGE_TOKEN_ATTR_NONE
    );

    assert_token(
        &cursor,
        " ",
        NO_HREF,
        PAGE_TOKEN_TEXT,
        PAGE_TOKEN_ATTR_BG_BLUE,
        PAGE_TOKEN_ATTR_BLUE,
        PAGE_TOKEN_ATTR_NONE
    );

    assert_token(
        &cursor,
        "                                    ",
        NO_HREF,
        PAGE_TOKEN_TEXT,
        PAGE_TOKEN_ATTR_BG_BLUE,
        PAGE_TOKEN_ATTR_BLUE,
        PAGE_TOKEN_ATTR_NONE
    );

    assert_token(
        &cursor,
        " ",
        NO_HREF,
        PAGE_TOKEN_TEXT,
        PAGE_TOKEN_ATTR_BG_BLACK,
        PAGE_TOKEN_ATTR_WHITE,
        PAGE_TOKEN_ATTR_NONE
    );

    assert_token(
        &cursor,
        " ",
        NO_HREF,
        PAGE_TOKEN_TEXT,
        PAGE_TOKEN_ATTR_BG_YELLOW,
        PAGE_TOKEN_ATTR_YELLOW,
        PAGE_TOKEN_ATTR_NONE
    );

    assert_token(
        &cursor,
        " ",
        NO_HREF,
        PAGE_TOKEN_TEXT,
        PAGE_TOKEN_ATTR_BG_YELLOW,
        PAGE_TOKEN_ATTR_YELLOW,
        PAGE_TOKEN_ATTR_NONE
    );

    assert_token(
        &cursor,
        " ",
        NO_HREF,
        PAGE_TOKEN_TEXT,
        PAGE_TOKEN_ATTR_BG_BLUE,
        PAGE_TOKEN_ATTR_BLUE,
        PAGE_TOKEN_ATTR_NONE
    );

    assert_token(
        &cursor,
        "                                    ",
        NO_HREF,
        PAGE_TOKEN_TEXT,
        PAGE_TOKEN_ATTR_BG_BLUE,
        PAGE_TOKEN_ATTR_BLUE,
        PAGE_TOKEN_ATTR_NONE
    );

    // TODO: Add more asserts

    page_destroy(page);
    error_reset();
}

int main() {
    if (
        !load_test_data(&JSON_DATA_PAGE, JSON_DATA_PAGE_PATH) ||
        !load_test_data(&JSON_DATA_PAGE_RANGE, JSON_DATA_PAGE_RANGE_PATH) ||
        !load_test_data(&JSON_DATA_PAGE_RANGE_LARGE, JSON_DATA_PAGE_RANGE_LARGE_PATH) ||
        !load_test_data(&HTML_DATA_PAGE_1, HTML_DATA_PAGE_1_PATH)
    ) {
        printf("Failed to load test JSON data!\n");
        exit(1);
    }

    CU_initialize_registry();
    CU_pSuite page_parser_suite = CU_add_suite("Page parser tests", 0, 0);
    CU_pSuite html_parser_suite = CU_add_suite("HTML parser tests", 0, 0);

    CU_add_test(page_parser_suite, "test_page_null_string", test_page_null_string);
    CU_add_test(page_parser_suite, "test_page_empty_string", test_page_empty_string);
    CU_add_test(page_parser_suite, "test_page_empty_array", test_page_empty_array);
    CU_add_test(page_parser_suite, "test_page_empty_title", test_page_empty_title);
    CU_add_test(page_parser_suite, "test_page_single_char_title", test_page_single_char_title);
    CU_add_test(page_parser_suite, "test_page_invalid_keys", test_page_invalid_keys);
    CU_add_test(page_parser_suite, "test_page_object_without_array", test_page_object_without_array);
    CU_add_test(page_parser_suite, "test_page_invalid", test_page_invalid);
    CU_add_test(page_parser_suite, "test_page_collection_empty_objects", test_page_collection_empty_objects);
    CU_add_test(page_parser_suite, "test_page_large_content_array", test_page_large_content_array);
    CU_add_test(page_parser_suite, "test_page_single", test_page_single);
    CU_add_test(page_parser_suite, "test_page_range", test_page_range);
    CU_add_test(page_parser_suite, "test_page_range_large", test_page_range_large);

    CU_add_test(html_parser_suite, "test_page_html_null", test_page_html_null);
    CU_add_test(html_parser_suite, "test_page_html_invalid_start_tag", test_page_html_invalid_start_tag);
    CU_add_test(html_parser_suite, "test_page_html_unexpected_end", test_page_html_unexpected_end);
    CU_add_test(html_parser_suite, "test_page_html_empty_class", test_page_html_empty_class);
    CU_add_test(html_parser_suite, "test_page_html_nested_a_tag", test_page_html_nested_a_tag);
    CU_add_test(html_parser_suite, "test_page_html_h1_tag", test_page_html_h1_tag);
    CU_add_test(html_parser_suite, "test_page_html_h1_tag_empty_class", test_page_html_h1_tag_empty_class);
    CU_add_test(html_parser_suite, "test_page_html_h1_tag_nested_a_tag", test_page_html_h1_tag_nested_a_tag);
    CU_add_test(html_parser_suite, "test_page_html_bold_text", test_page_html_bold_text);
    CU_add_test(html_parser_suite, "test_page_html_newline_no_whitespace", test_page_html_newline_no_whitespace);
    CU_add_test(html_parser_suite, "test_page_html_newline_whitespace", test_page_html_newline_whitespace);
    CU_add_test(html_parser_suite, "test_page_html_span_separator", test_page_html_span_separator);
    CU_add_test(html_parser_suite, "test_page_html_1", test_page_html_1);

    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    CU_cleanup_registry();

    destroy_test_data(&JSON_DATA_PAGE);
    destroy_test_data(&JSON_DATA_PAGE_RANGE);
    destroy_test_data(&JSON_DATA_PAGE_RANGE_LARGE);
    destroy_test_data(&HTML_DATA_PAGE_1);

    return 0;
}
