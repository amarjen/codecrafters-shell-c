#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <stdarg.h>
#include "colors.h"
#include "utils.h"

#define USE_READLINE 0
#define USE_LINENOISE 0

#define PS1 BHGRN "$ " CRESET
#define DEBUG 0

const char *builtin[] = {"exit", "echo", "type", "pwd", "cd"};
int builtin_size = sizeof(builtin) / sizeof(builtin[0]);

char exit_command[]="exit";
char echo_command[]="echo";
char type_command[]="type";
char pwd_command[]="pwd";
char cd_command[]="cd";

int main()
{
    // Flush after every printf
    setbuf(stdout, NULL);

    int running = 1;

    // char* input;
    char input[100];

#if USE_READLINE == 1
#include <readline/readline.h>
#include <readline/history.h>

    while ((input = readline(PS1)) != NULL) {
        if (strlen(input) > 0) {
            add_history(input);
        }

#elif USE_LINENOISE == 1
#include "linenoise.h"

    while ((input = linenoise(PS1)) != NULL) {
#else

    while (running) {
        //1. Reads input
        printf(PS1);
        fgets(input, 100, stdin);

        if (input[0]=='\n') {
            continue;
        }
        input[strcspn(input, "\n")] = '\0';
#endif

        char **tokens = (char**)ecalloc(10, sizeof(char*));
        int token_count = 0;

        // 2. Break the input into tokens (words and operators)
        tokenize(input, tokens, &token_count);
        if (DEBUG) {
            for (int i=0; i<token_count; i++) {
                printf("Tk %d: %s\n", i, tokens[i]);
            }
        }

        performExpansions(tokens, token_count);

        Command cmd;
        parseCommand(tokens, token_count, &cmd);
        if (DEBUG) {
            printCommand(&cmd);
        }

        // EXIT COMMAND
        if (strcmp(cmd.command, exit_command) == 0) {
            running=0;
            freeTokens(tokens, token_count);
            break;
        }

        // // ECHO COMMAND
        // else if (strcmp(cmd.command, echo_command) == 0) {
        //   }
        // }
        //

        // TYPE COMMAND
        else if (strcmp(cmd.command, type_command) == 0) {
            if (token_count == 1) {
                printf("You need to specify a command\n");
            } else {
                char* fp;
                fp = inPath(tokens[1]);

                if (inArray(tokens[1], builtin, builtin_size) == 1) {
                    printf("%s is a shell builtin\n", tokens[1]);
                }

                else if (fp != NULL) {
                    printf("%s is %s\n", tokens[1], fp);
                } else {
                    printf("%s: not found\n", tokens[1]);
                }

                free(fp);
            }
        }

        // PWD COMMAND
        else if (strcmp(cmd.command, pwd_command) == 0) {
            char cwd[1024];
            getcwd(cwd, sizeof(cwd));
            printf("%s\n",cwd);
        }

        // CD COMMAND
        else if (strcmp(cmd.command, cd_command) == 0) {
            if (token_count == 1) {
                printf("You need to specify a directory\n");
            } else {
                int result = chdir(cmd.args[1]);
                if ((result != 0) && (errno == ENOENT)) {
                    printf("cd: %s: No such file or directory\n", cmd.args[1]);
                }
            }
        }

        // EXECUTE COMMAND IN PATH
        else { // if ( inPath(cmd.command) != NULL )
            char* fp;
            fp = inPath(cmd.command);

            if (fp != NULL) {
                free(fp);
                if (parseCommand(tokens, token_count, &cmd) == 0) {
                    pid_t pid = fork();
                    if (pid == 0) {
                        // Child process: Execute the command
                        executeCommand(&cmd);
                    } else if (pid > 0) {
                        // Parent process: Wait for the child to finish
                        wait(NULL);
                    } else {
                        perror("Error creating child process");
                        exit(EXIT_FAILURE);
                    }
                }
            }

            else {
                free(fp);
                printf("%s: command not found\n", cmd.command);
            }
        }

        freeTokens(tokens, token_count);

#if USE_READLINE == 1
        // readline malloc's a new buffer every time.
        free(input);
#endif /* if USE_READLINE */

    }

    return 0;
}
