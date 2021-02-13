#include "html_parser.h"

#define MAX_TOKENS              256
#define DIV_TAG_START_LENGTH    20  // <div class=\"root\"> = 20 chars
#define DIV_TAG_END_LENGTH      9   // \n<\/div>
#define SPAN_TAG_CLASS_OFFSET   13  // <span class="

/// @brief Removes unnecessary backslashes and div-tag
static void clean_content_junk(char *buf, const char *html_content, size_t size) {
    int buf_position = 0;
    
    // Make sure we dont read incorrectly or cause SIGSEGV
    if (!html_content || size <= 26) {
        buf[0] = '\0';
        return;
    }
    
    // Skip first and last characters to remove div-tag
    for (int i = DIV_TAG_START_LENGTH; i < (size - DIV_TAG_END_LENGTH); i++, buf_position++) {
        // Only remove backslashes that are not part of the newline escape sequence
        if (html_content[i] == '\\' && html_content[i + 1] != 'n') {
            // Move to next non-backslash character
            i++; 
        } 
        
        buf[buf_position] = html_content[i];
    }
    
    buf[buf_position] = '\0';
}

static void parse_span_tag(page_token_t **tokens_cursor, char **cursor, size_t *size) {
    page_token_style_t style = {
        .fg = PAGE_TOKEN_ATTR_WHITE,
        .bg = PAGE_TOKEN_ATTR_BG_BLACK,
        .extra = PAGE_TOKEN_ATTR_NONE
    };
    
    page_token_t token = {
        .text = NULL,
        .size = 0,
        .type = PAGE_TOKEN_ATTR_NONE,
        .style = style
    };

    char *span_content = *cursor;
    
    if (span_content[0] != '<' || span_content[1] != 's') {
        error_set_with_string(
            TTT_ERROR_HTML_PARSER_FAILED,
            "ERROR: Invalid HTML content string, expected span-tag"
        );
        
        // Set size to tell parser that no more tokens can be parsed
        *size = 0;
        return;
    }

    // Add token to tokens array
    **tokens_cursor = token;
    
    // Move to next positions in the tokens array
    (*tokens_cursor) += 1;

    // Move to first character in span class attribute
    (*cursor) += SPAN_TAG_CLASS_OFFSET;
    
    int i = 0;
    char class_buf[32];
    
    // Extract each classname, separated by a space
    while (**cursor != '"') {
        if (**cursor == ' ') {
            // We have a complete classname in class_buf, add token attribute
            if (strncmp(class_buf, "toprow", i) == 0) {
            }
            
            // Reset class_buf
            i = 0;
        } else {
            printf("%c ", (*cursor)[i]);
            class_buf[i] = **cursor;
            i++;
        }
        
        (*cursor) += 1;
        *size -= 1;
    }
}

page_token_t *html_parser_get_page_tokens(const char *html, size_t size) {
    if (!html || size == 0) {
        error_set_with_string(
            TTT_ERROR_HTML_PARSER_FAILED,
            "ERROR: Could not parse empty HTML page content"
        );
        return NULL;
    }

    int token_count = 0;
    char buf[size];
    char *cursor = buf;
    
    page_token_t tokens[MAX_TOKENS];
    page_token_t *tokens_cursor = tokens;
    
    clean_content_junk(buf, html, size);
    
    while (size > 0) {
        parse_span_tag(&tokens_cursor, &cursor, &size);
        token_count += 1;
    }
   
    if (token_count == 0) {
        // Parsing failed
        return NULL;
    }
   
    page_token_t *page_tokens = calloc(token_count, sizeof(page_token_t));
    
    for (int i = 0; i < token_count; i++) {
        page_tokens[i] = tokens[i];
    }
   
    return page_tokens;
}