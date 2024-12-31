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
    cmd = strtok(input, s);
    // char tokenized_input[] = strok(input, " ");

    char exit_command[]="exit";
    char echo_command[]="echo";

    if (strcmp(cmd, exit_command) == 0) {
      running=0;
      break;
    }

    char* args;
    args = strtok(0, s);
    if (strcmp(input, echo_command) == 0) {
      while (args != 0) {
        printf("%s ", args);
        args = strtok(0, s);
      }
      printf("\n");
    }

    else {
      printf("%s: command not found\n", input);
    }
  }
  return 0;
}
