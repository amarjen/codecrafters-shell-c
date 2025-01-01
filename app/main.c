#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h> 
#include <unistd.h>
#include <errno.h>

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
  char *path_copy = strndup(path, strlen(path)); 
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

void trim(char *s) {
    // Two pointers initially at the beginning
    int i = 0, j = 0;

    // Skip leading spaces
    while (s[i] == ' ') i++; 

    // Shift the characters of string to remove
    // leading spaces
    while (s[j++] = s[i++]);
}

int tokenize(char **tokens, char *str) {
    char separator[2] = " ";
    char* next_token;
    tokens[0] = strtok(str, separator);
    next_token = strtok(0, separator);
    int i=1;
    while (next_token != 0) {
      tokens[i] = malloc(strlen(next_token)+1);
      strcpy(tokens[i], next_token);
      next_token = strtok(0, separator);
      i++;
    }
    return i;
}

static char *util_cat(char *dest, char *end, const char *str)
{
    while (dest < end && *str)
        *dest++ = *str++;
    return dest;
}

size_t join_str(char *out_string, size_t out_bufsz, const char *delim, char **chararr)
{
    // saltar el primer elemento para que queden solo los argumentos
    chararr++;
  
    char *ptr = out_string;
    char *strend = out_string + out_bufsz;
    while (ptr < strend && *chararr)
    {
        ptr = util_cat(ptr, strend, *chararr);
        chararr++;
        if (*chararr)
            ptr = util_cat(ptr, strend, delim);
    }
    return ptr - out_string;
}

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
  int running = 1;
  while (running) {
    printf("$ ");

    // Wait for user input
    char input[100];
    fgets(input, 100, stdin);

    input[strcspn(input, "\n")] = '\0';

    char **argv = (char**)malloc(5*sizeof(char*));
    int argc = tokenize(argv, input);

    char *args = malloc(argc*sizeof(char*)+argc);
    join_str(args, argc * sizeof(char*), s, argv);

    char* cmd;
    cmd = argv[0];
    if (cmd == NULL) { continue; }

    // EXIT COMMAND
    if (strcmp(cmd, exit_command) == 0) {
      running=0;
      break;
    }

    // ECHO COMMAND
    else if (strcmp(cmd, echo_command) == 0) {
      char target = '\'';
      const char* result = args;
      while((result = strchr(result, target))!=NULL) {
        printf("Found '%c' starting at '%s'\n", target, result);
        ++result;
      }
      printf("%s\n", args);
    }

    // TYPE COMMAND
    else if (strcmp(cmd, type_command) == 0) {

        if (inArray(args, commands, commands_size) == 1) {
           printf("%s is a shell builtin\n", args);
           }

        else if (inPath(args) != NULL) {
           char* fp;
           fp = inPath(args);
           printf("%s is %s\n", args, fp);
           free(fp);
      }
        else { printf("%s: not found\n", args);}
    }

    else if (strcmp(cmd, pwd_command) == 0) {
      char cwd[1024];
      getcwd(cwd, sizeof(cwd));
      printf("%s\n",cwd);
    }

    else if (strcmp(cmd, cd_command) == 0) {
      if (!strcmp(args, "~")) {
        strcpy(args, getenv("HOME"));
      }

      int result = chdir(args);
      if ((result != 0) && (errno == ENOENT)) {
        printf("cd: %s: No such file or directory\n", args);
      }
    }

    // EXECUTE COMMAND IN PATH
    else if ( inPath(cmd) != NULL ) {

      char* fullCommand = malloc(100);
      if (args != NULL) {
        snprintf(fullCommand, 100, "%s %s\n",cmd, args);
      }
      else {
        snprintf(fullCommand, 100, "%s\n",cmd);
      }
      int returnCode = system(fullCommand);
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
