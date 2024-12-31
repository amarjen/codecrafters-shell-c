#include <stdio.h>
#include <string.h>

int main() {
  // Flush after every printf
  setbuf(stdout, NULL);

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

    char commands[][20] = {"exit", "echo", "type"};
    char exit_command[]="exit";
    char echo_command[]="echo";
    char type_command[]="type";

    // EXIT COMMAND
    if (strcmp(cmd, exit_command) == 0) {
      running=0;
      break;
    }

    // ECHO COMMAND
    if (strcmp(cmd, echo_command) == 0) {
      while (arg != 0) {
        printf("%s ", arg);
        arg = strtok(0, s);
      }
      printf("\n");
    }

    // TYPE COMMAND
    if (strcmp(cmd, type_command) == 0) {
      int found = 0;
      for (int i=0; i<3; i++) {
        if (strcmp(arg, commands[i]) == 0) {
           printf("%s is a shell builtin\n", arg);
          found = 1;
           }
      }
      if (!found) { printf("%s: command not found\n", cmd);}
    }

    else {
      printf("%s: command not found\n", input);
    }
  }
  return 0;
}
