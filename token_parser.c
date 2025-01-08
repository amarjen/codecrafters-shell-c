
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_ARGS 100 // Maximum number of arguments

typedef struct {
    char *command;         // Command name
    char *args[MAX_ARGS];  // Arguments
    int argc;              // Argument count
    int redirect_out_fd;   // File descriptor for `>` or `>>`
    char *redirect_out;    // Output redirection file
    int append_out;        // 1 for append (>>), 0 for overwrite (>)
} Command;

int parseCommand(char **tokens, int token_count, Command *cmd) {
    if (token_count == 0) {
        fprintf(stderr, "Error: Empty token list\n");
        return -1;
    }

    memset(cmd, 0, sizeof(Command)); // Initialize the Command structure

    // State 1: Parse command name
    cmd->command = tokens[0];
    cmd->argc = 0;

    for (int i = 1; i < token_count; i++) {
        char *token = tokens[i];

        if (strcmp(token, ">") == 0 || strcmp(token, ">>") == 0 || (isdigit(token[0]) && (strcmp(token + 1, ">") == 0 || strcmp(token + 1, ">>") == 0))) {
            // State 2: Parse redirection operator
            int fd = isdigit(token[0]) ? token[0] - '0' : 1; // Default to stdout
            int append = (strstr(token, ">>") != NULL);

            if (i + 1 >= token_count) {
                fprintf(stderr, "Error: Missing filename after redirection\n");
                return -1;
            }

            cmd->redirect_out_fd = fd;
            cmd->redirect_out = tokens[++i];
            cmd->append_out = append;
        } else {
            // State 3: Parse arguments
            if (cmd->argc >= MAX_ARGS) {
                fprintf(stderr, "Error: Too many arguments\n");
                return -1;
            }
            cmd->args[cmd->argc++] = token;
        }
    }

    return 0; // Success
}

void printCommand(const Command *cmd) {
    printf("Command: %s\n", cmd->command);
    printf("Arguments (%d):\n", cmd->argc);
    for (int i = 0; i < cmd->argc; i++) {
        printf("  [%d]: %s\n", i, cmd->args[i]);
    }

    if (cmd->redirect_out) {
        printf("Redirection:\n");
        printf("  File descriptor: %d\n", cmd->redirect_out_fd);
        printf("  File: %s\n", cmd->redirect_out);
        printf("  Append: %s\n", cmd->append_out ? "Yes" : "No");
    }
}

int main() {
    // Example input
    char *tokens[] = {"cmd", "arg1", "arg2", "1>", "output.txt", "2>>", "error.log"};
    int token_count = 7;

    Command cmd;
    if (parseCommand(tokens, token_count, &cmd) == 0) {
        printCommand(&cmd);
    }

    return 0;
}
