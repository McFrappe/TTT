#include "html_parser.h"

#define DIV_TAG_START_LENGTH    20  // <div class=\"root\"> = 20 chars
#define DIV_TAG_END_LENGTH      9   // \n<\/div>


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

page_token_t **html_parser_get_page_tokens(const char *html, size_t size) {
    if (!html || size == 0) {
        error_set_with_string(
            TTT_ERROR_HTML_PARSER_FAILED,
            "ERROR: Could not parse empty HTML page content"
        );
        return NULL;
    }

    char buf[size];
    clean_content_junk(buf, html, size);
    
    //printf("%s\n", buf);
    return NULL;
}