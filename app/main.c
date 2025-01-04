#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
  char quote[2] = "'";

  int running = 1;
  while (running) {
    printf(BHGRN "$ " CRESET);

    // Wait for user input
    char input[100];
    printf(CYN);
    fgets(input, 100, stdin);
    printf(CRESET);

    input[strcspn(input, "\n")] = '\0';

    char **argv = (char**)ecalloc(5, sizeof(char*));
    char *args = ecalloc(8, 1024);
    char *args_quotes = ecalloc(8, 1024);
    int argc = 0;
    char cmd[256];
 
    //parse_line(input, cmd, argv, &argc);
    parse_tokens(input, argv, &argc);
    strcpy(cmd, argv[0]);

    if (argc > 1) {
      join_str(args, 1024, s, argv);

      for (int i=1; i<argc; i++){
        strcat(args_quotes, "'");
        strcat(args_quotes, argv[i] );
        strcat(args_quotes, "'");
        strcat(args_quotes, " ");
      }
    }
    else {
      args = NULL;
      args_quotes = NULL;
    }

    if (DEBUG) {
      for (int i=0; i<argc; i++) {
        printf("argv %d: %s\n", i, argv[i]);
      }
      printf("cmd: %s\nargc: %d\nargs: %s\nargsq: %s\n---\n", cmd, argc, args,args_quotes);
      }
    if (cmd == NULL) { continue; }

    // EXIT COMMAND
    if (strcmp(cmd, exit_command) == 0) {
      running=0;
      break;
    }

    // ECHO COMMAND
    else if (strcmp(cmd, echo_command) == 0) {
      if (args != NULL) {
      printf("%s\n", args);}
    }

    // TYPE COMMAND
    else if (strcmp(cmd, type_command) == 0) {
      if (argc == 0) {printf("You need to specify a command\n");
      }
      else {

        if (inArray(argv[1], commands, commands_size) == 1) {
           printf("%s is a shell builtin\n", argv[1]);
           }

        else if (inPath(argv[1]) != NULL) {
           char* fp;
           fp = inPath(argv[1]);
           printf("%s is %s\n", argv[1], fp);
           free(fp);
      }
        else { printf("%s: not found\n", argv[1]);}
      }   }

    // PWD COMMAND
    else if (strcmp(cmd, pwd_command) == 0) {
      char cwd[1024];
      getcwd(cwd, sizeof(cwd));
      printf("%s\n",cwd);
    }

    else if (strcmp(cmd, cd_command) == 0) {
      if (argc == 0) {printf("You need to specify a directory\n");
      }
      else {
        if (!strcmp(argv[1], "~")) {
          strcpy(args, getenv("HOME"));
        }

        int result = chdir(args);
        if ((result != 0) && (errno == ENOENT)) {
           printf("cd: %s: No such file or directory\n", args);
        }
      }
    }

    // EXECUTE COMMAND IN PATH
    else if ( inPath(cmd) != NULL ) {

      char* fullCommand = ecalloc(1, 100);
      if (args_quotes != NULL) {
        snprintf(fullCommand, 100, "%s %s\n",cmd, args_quotes);
      }
      else {
        snprintf(fullCommand, 100, "%s\n",cmd);
      }
      int returnCode = system(fullCommand);
      // pid_t pid = fork();
      //
      // if (pid == 0) { // child
      //
      //   char *argsp[] = {cmd, NULL};
      //   execvp(cmd, argsp);
      //
      // } else { // parent
      //
      //   int status;
      //   waitpid(pid, &status, 0);
      //
      //   printf("Child process finished\n");
      // }
        free(fullCommand);
    }

    else {
      printf("%s: command not found\n", input);
    }

    free(argv);
    free(args);
  }
  return 0;
}
