
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_ARGS 100    // Maximum number of arguments
#define MAX_ARG_LEN 256 // Maximum length of each argument

void parse_line(const char *line, char *command_output, char **args_output, int *arg_count) {
    const char *ptr = line; // Pointer to traverse the input string
    char *arg = NULL;
    int index = 0;

    // Skip leading spaces
    while (*ptr && isspace((unsigned char)*ptr)) {
        ptr++;
    }

    // Extract the command (first token)
    while (*ptr && !isspace((unsigned char)*ptr)) {
        *command_output++ = *ptr++;
    }
    *command_output = '\0'; // Null-terminate the command string

    // Parse the arguments
    while (*ptr) {
        // Skip leading spaces
        while (*ptr && isspace((unsigned char)*ptr)) {
            ptr++;
        }

        if (*ptr == '\0') {
            break; // End of string
        }

        if (*ptr == '\'') { // Handle quoted argument
            ptr++; // Skip the opening single quote
            arg = malloc(MAX_ARG_LEN);
            int arg_pos = 0;

            while (*ptr && *ptr != '\'') {
                if (arg_pos < MAX_ARG_LEN - 1) {
                    arg[arg_pos++] = *ptr++;
                } else {
                    fprintf(stderr, "Argument exceeds maximum length\n");
                    free(arg);
                    return;
                }
            }

            if (*ptr == '\'') {
                ptr++; // Skip the closing single quote
            }
            arg[arg_pos] = '\0';
        } else { // Handle unquoted argument
            arg = malloc(MAX_ARG_LEN);
            int arg_pos = 0;

            while (*ptr && !isspace((unsigned char)*ptr)) {
                if (arg_pos < MAX_ARG_LEN - 1) {
                    arg[arg_pos++] = *ptr++;
                } else {
                    fprintf(stderr, "Argument exceeds maximum length\n");
                    free(arg);
                    return;
                }
            }
            arg[arg_pos] = '\0';
        }

        // Add the argument to the output array
        if (index < MAX_ARGS) {
            args_output[index++] = arg;
        } else {
            fprintf(stderr, "Too many arguments\n");
            free(arg);
            return;
        }
    }

    *arg_count = index; // Update the argument count
}

int main() {
    const char *line = "command arg1   arg3 'algo  que     ver'";
    char command[256];
    char *args[MAX_ARGS];
    int arg_count = 0;

    parse_line(line, command, args, &arg_count);

    printf("Command: %s\n", command);
    printf("Arguments (%d):\n", arg_count);
    for (int i = 0; i < arg_count; i++) {
        printf("  [%d]: %s\n", i, args[i]);
        free(args[i]); // Free dynamically allocated memory for each argument
    }

    return 0;
}
