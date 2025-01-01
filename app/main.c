#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h> 

int inArray(const char *str, const char *arr[], int size) {
  if (str == NULL) {
    return 0;
  }
  for (int i=0; i<size; i++) {
    if (strcmp(str, arr[i]) == 0) {
      return 1; // Found
    }
  }
  return 0; // Not Found
}

char* inPath(const char *command) {

  char* path = getenv("PATH");
  char *path_copy = strdup(path); 
  const char split[2] = ":";
  struct stat file_stat;
  
  char *token;
  token = strtok(path_copy, split);

  while (token != NULL) {
    char *fullpath = malloc(100);
    snprintf(fullpath, 100, "%s/%s", token, command);

    if ( (stat(fullpath, &file_stat) == 0) && (file_stat.st_mode & S_IXOTH )) {
      free(path_copy);
      return fullpath;
    }
    token = strtok(NULL, split);
    free(fullpath);
  }
  free(path_copy);
  return NULL;
}

int main() {
  // Flush after every printf
  setbuf(stdout, NULL);

  const char *commands[] = {"exit", "echo", "type"};
  int commands_size = sizeof(commands) / sizeof(commands[0]);

  char exit_command[]="exit";
  char echo_command[]="echo";
  char type_command[]="type";

  int running = 1;
  while (running) {
    printf("$ ");

    // Wait for user input
    char input[100];
    fgets(input, 100, stdin);

    input[strcspn(input, "\n")] = ' ';

    const char s[4] = " ";
    char* cmd;
    char* arg;
    cmd = strtok(input, s);
    arg = strtok(0, s);

    if (cmd == NULL) { continue; }

    // EXIT COMMAND
    if (strcmp(cmd, exit_command) == 0) {
      running=0;
      break;
    }

    // ECHO COMMAND
    else if (strcmp(cmd, echo_command) == 0) {
      while (arg != 0) {
        printf("%s ", arg);
        arg = strtok(0, s);
      }
      printf("\n");
    }

    // TYPE COMMAND
    else if (strcmp(cmd, type_command) == 0) {
        char *fp = inPath(arg);

        if (inArray(arg, commands, commands_size) == 1) {
           printf("%s is a shell builtin\n", arg);
           }

        else if (fp != NULL) {
           printf("%s is %s\n", arg, fp);

      }
        else { printf("%s: not found\n", arg);}
      free(fp);
    }

    else {
      printf("%s: command not found\n", input);
    }
  }
  return 0;
}
