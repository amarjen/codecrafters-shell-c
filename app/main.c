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

    input[strcspn(input, "\n")] = '\0';

    char exit_command[]="exit 0";

    if (strcmp(input, exit_command) == 0) {
      running=0;
      break;
    }

    else {
      printf("%s: command not found\n", input);
    }
  }
  return 0;
}
