
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_TOKENS 100 // Maximum number of tokens
#define MAX_TOKEN_LEN 256 // Maximum token length

enum states { DEFAULT, SINGLE_Q, DOUBLE_Q } state = DEFAULT;

int parse_tokens(const char *input, char **tokens, int *token_count) {
    // https://www.gnu.org/software/bash/manual/bash.html#Quoting
    int is_escaped = 0;
    int in_single_quote = 0;
    int in_double_quote = 0;

    char buffer[MAX_TOKEN_LEN];
    int buffer_pos = 0;
    int token_index = 0;

    for (p = input; *p != '\0'; p++) {
        c = (unsigned char) *p; /* convert to unsigned char for is* functions */
        switch (state) {

    while (*input) {

        if (is_escaped) { // Handle escaped character
            buffer[buffer_pos++] = c;
            is_escaped = 0;
        }

        // SINGLE QUOTES
        else if (c == '\'' && !in_double_quote) { // Single quote toggle
            in_single_quote = !in_single_quote;
        }

        // DOUBLE QUOTES
        else if (c == '"' && !in_single_quote) { // Double quote toggle
            in_double_quote = !in_double_quote;
        }

        else if (in_double_quote && c == '\\') { 
          // Enclosing backslashes within double quotes " preserves
          // the special meaning of the backslash, only when it is followed by \, $, " or newline.

           char next_c = *input++; // mirar el siguiente y retroceder
           if (next_c != '\\' || next_c != '$' || next_c != '"' || next_c != 'n') {
             /*buffer[buffer_pos++] = c;*/
             buffer[buffer_pos++] = next_c;
             /**input++;*/
           }
           else {
             *input--;
             buffer[buffer_pos++] = c;
           /*is_escaped = 1;*/
           }
        }

        else if (c == '\\' && !in_single_quote && !in_double_quote) { // Escape character
            is_escaped = 1;
        }


        else if (isspace((unsigned char)c) && !in_single_quote && !in_double_quote) { // Token boundary
            if (buffer_pos > 0) {
                buffer[buffer_pos] = '\0'; // Null-terminate the token
                tokens[token_index++] = strdup(buffer); // Save the token
                buffer_pos = 0;

                if (token_index >= MAX_TOKENS) {
                    fprintf(stderr, "Error: Too many tokens\n");
                    return -1;
                }
            }
        }

        else { // Regular character
            buffer[buffer_pos++] = c;
            if (buffer_pos >= MAX_TOKEN_LEN) {
                fprintf(stderr, "Error: Token exceeds maximum length\n");
                return -1;
            }
        }
    }

    // Save the last token if any
    if (buffer_pos > 0) {
        buffer[buffer_pos] = '\0';
        tokens[token_index++] = strdup(buffer);
    }

    // Check for unmatched quotes
    if (in_single_quote || in_double_quote) {
        fprintf(stderr, "Error: Unmatched quote\n");
        return -1;
    }

    *token_count = token_index;
    return 0;
}


int main() {
    // const char *input = "command \"arg w'ith spaces\" 'another arg' escaped space'bla''ble'";
    const char *input = "\"/tmp/qux/f 6\" \"/tmp/qux/f   33\" \"/tmp/qux/f's60\"";
    printf("Input: %s\n", input);
    char *tokens[MAX_TOKENS];
    int token_count = 0;

    if (parse_tokens(input, tokens, &token_count) == 0) {
        printf("Parsed tokens (%d):\n", token_count);
        for (int i = 0; i < token_count; i++) {
            printf("  [%d]: %s\n", i, tokens[i]);
            free(tokens[i]); // Free allocated memory
        }
    }

    return 0;
}
