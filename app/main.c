#include <stdio.h>
#include <string.h>

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

int main() {
  // Flush after every printf
  setbuf(stdout, NULL);

  const char *commands[] = {"exit", "echo", "type"};
  int commands_size = sizeof(commands) / sizeof(commands[0]);

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

    char exit_command[]="exit";
    char echo_command[]="echo";
    char type_command[]="type";

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
        if (inArray(arg, commands, commands_size) == 1) {
           printf("%s is a shell builtin\n", arg);
           }
        else { printf("%s: not found\n", arg);}
    }

    else {
      printf("%s: command not found\n", input);
    }
  }
  return 0;
}
