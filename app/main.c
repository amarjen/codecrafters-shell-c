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

#define DEBUG 0 // Change to 1 to verbose debug prints

// SHELL LOOP
int main() {
  // Flush after every printf
  setbuf(stdout, NULL);

  const char *commands[] = {"exit", "echo", "type", "pwd", "cd"};
  int commands_size = sizeof(commands) / sizeof(commands[0]);

  char exit_command[]="exit";
  char echo_command[]="echo";
  char type_command[]="type";
  char pwd_command[]="pwd";
  char cd_command[]="cd";

  char s[2] = " ";
  char input[100];

  int running = 1;
  while (running) {
    //1. Reads input
    printf(BHGRN "$ " CRESET);

    printf(CYN);
    fgets(input, 100, stdin);
    printf(CRESET);

    if (input[0]=='\n') {continue;}
    input[strcspn(input, "\n")] = '\0';

    char **tokens = (char**)ecalloc(10, sizeof(char*));
    char *args = ecalloc(8, 1024);
    char args_quotes[1024] = {0};
    int token_count = 0;
 
    // 2. Break the input into tokens (words and operators)
    tokenize(input, tokens, &token_count);
    if (DEBUG) {
      for (int i=0;i<token_count;i++) {
        printf("Tk %d: %s\n", i, tokens[i]);
      }
    }

    performExpansions(tokens, token_count);

    Command cmd;
    parseCommand(tokens, token_count, &cmd);
    if (DEBUG) {printCommand(&cmd);}


    if (token_count > 1) {
      join_str(args, 1024, " ", cmd.args);
      expandArgs(tokens, token_count, args_quotes);
    }
    else {
      args = NULL;
      // args_quotes = NULL;
    }

    if (DEBUG){
      printf("args: %s\n", args);
      printf("argsq: %s\n", args_quotes);
    }


    // EXIT COMMAND
    if (strcmp(cmd.command, exit_command) == 0) {
      running=0;
      break;
    }

    // // ECHO COMMAND
    // else if (strcmp(cmd.command, echo_command) == 0) {
    //   if (args != NULL) {
    //   printf("%s\n", args);
    //   }
    // }
    //
    // TYPE COMMAND
    else if (strcmp(cmd.command, type_command) == 0) {
      if (token_count == 0) {printf("You need to specify a command\n");
      }
      else {

        if (inArray(tokens[1], commands, commands_size) == 1) {
           printf("%s is a shell builtin\n", tokens[1]);
           }

        else if (inPath(tokens[1]) != NULL) {
           char* fp;
           fp = inPath(tokens[1]);
           printf("%s is %s\n", tokens[1], fp);
           free(fp);
      }
        else { printf("%s: not found\n", tokens[1]);}
      }   }

    // PWD COMMAND
    else if (strcmp(cmd.command, pwd_command) == 0) {
      char cwd[1024];
      getcwd(cwd, sizeof(cwd));
      printf("%s\n",cwd);
    }

    // CD COMMAND
    else if (strcmp(cmd.command, cd_command) == 0) {
      if (token_count == 0) { 
        printf("You need to specify a directory\n");
      }
      else {
        int result = chdir(cmd.args[1]);
        if ((result != 0) && (errno == ENOENT)) {
           printf("cd: %s: No such file or directory\n", args);
        }
      }
    }

    // EXECUTE COMMAND IN PATH
    else if ( inPath(cmd.command) != NULL ) {
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

      // char* fullCommand = ecalloc(1, 100);
      //
      // if ( args != NULL) {
      //
      //   if (input[0] == '\"') {
      //     quoteStr(cmd.command);
      //   }
      //   else if (input[0] == '\'') {
      //     singlequoteStr(cmd.command);
      //   } 
      //
      //   snprintf(fullCommand, 1024, "%s %s\n",cmd.command, args_quotes);
      // }
      // else {
      //   snprintf(fullCommand, 1024, "%s\n",cmd.command);
      // }
      // int returnCode = system(fullCommand);
      //   free(fullCommand);
    }

    else {
      printf("%s: command not found\n", input);
    }
    free(tokens);
    free(args);
 
 }

  return 0;
}
