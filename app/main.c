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
  const char split[4] = ":";
  static char fullpath[100];
  struct stat file_stat;

  path = strtok(path, split);

  while (path != 0) {
    strcpy(fullpath, path);
    strcat(fullpath, "/");
    strcat(fullpath, command);
    stat(fullpath, &file_stat);

    if ( file_stat.st_mode & S_IXOTH == 1 ) {
      printf("is Exec: %s\n", fullpath);
      return fullpath;
    }
    path = strtok(0, split);
  }
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
        char *fp;
        strcpy(fp, inPath(arg));

        if (inArray(arg, commands, commands_size) == 1) {
           printf("%s is a shell builtin\n", arg);
           }

        else if (fp != NULL) {
           printf("%s is %s\n", fp);

      }
        else { printf("%s: not found\n", arg);}
    }

    else {
      printf("%s: command not found\n", input);
    }
  }
  return 0;
}
