#include <stdio.h>
#include <string.h>

int main() {
  // Flush after every printf
  setbuf(stdout, NULL);

  int running = 1;
  while (running) {
  // Uncomment this block to pass the first stage
  printf("$ ");

  // Wait for user input
  char input[100];
  fgets(input, 100, stdin);

  input[strcspn(input, "\n")] = '\0';

  char quit_command[]="quit";

  if (strcmp(input, quit_command) == 0) {running=0; break;}
  else { printf("%s: command not found\n", input); }
  }
  return 0;
}
