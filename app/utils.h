#define MAX_ARGS 100    // Maximum number of arguments
#define MAX_ARG_LEN 256 // Maximum length of each argument


static void
die(const char *fmt, ...)
{
        va_list ap;
        va_start(ap, fmt);
        vfprintf(stderr, fmt, ap);
        va_end(ap);
        if (fmt[0] != '\0' && fmt[strlen(fmt) - 1] == ':') {
                fputc(' ', stderr);
                perror(NULL);
        } else {
                fputc('\n', stderr);
        }
        exit(EXIT_FAILURE);
}

static void *
ecalloc(size_t nmemb, size_t size)
{
        void *p;
        if ((p = calloc(nmemb, size)) == NULL)
                die("calloc:");
        return p;
}

static void *
erealloc(void *p, size_t len)
{
        if ((p = realloc(p, len)) == NULL)
                die("realloc: %s\n", strerror(errno));
        return p;
}

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
  strcat(path_copy, ":.");
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


static char *util_cat(char *dest, char *end, const char *str)
{
    while (dest < end && *str)
        *dest++ = *str++;
    return dest;
}

size_t join_str(char *out_string, size_t out_bufsz, char *delim, char **chararr)
{
    ++chararr; // skip first arg (executable filename)
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

#define MAX_TOKENS 100 // Maximum number of tokens
#define MAX_TOKEN_LEN 256 // Maximum token length

int tokenize(const char *input, char **tokens, int *token_count) {
    // Convierte input en tokens
    // https://www.gnu.org/software/bash/manual/bash.html#Quoting
    int is_escaped = 0;
    int in_single_quote = 0;
    int in_double_quote = 0;

    char buffer[MAX_TOKEN_LEN];
    int buffer_pos = 0;
    int token_index = 0;

    while (*input) {
        char c = *input++;

        if (is_escaped) { // Handle escaped character
            buffer[buffer_pos++] = c;
            is_escaped = 0;
        }

        // SINGLE QUOTES
        else if (c == '\'' && !in_double_quote) { // Single quote toggle
            in_single_quote = !in_single_quote;
        }

        // DOUBLE QUOTES
        else if (c == '"' && !in_single_quote) { // Double quote toggle
            in_double_quote = !in_double_quote;
        }

        else if (in_double_quote && c == '\\') { 
          // Enclosing backslashes within double quotes " preserves
          // the special meaning of the backslash, only when it is followed by \, $, " or newline.

           char next_c = *input++; // mirar el siguiente y retroceder
           if (next_c == '\\' || next_c == '"' ) {
             buffer[buffer_pos++] = next_c;
           } 
           else if (next_c != '$' ||  next_c != 'n') {
             buffer[buffer_pos++] = '\\';
             buffer[buffer_pos++] = next_c;
           }
           else {
             buffer[buffer_pos++] = c;
           }
        }

        else if (c == '\\' && !in_single_quote && !in_double_quote) { // Escape character
            is_escaped = 1;
        }

        else if (isspace((unsigned char)c) && !in_single_quote && !in_double_quote) { // Token boundary
            if ( buffer_pos > 0) {
                buffer[buffer_pos] = '\0'; // Null-terminate the token
                tokens[token_index++] = strdup(buffer); // Save the token
                buffer_pos = 0;

                if (token_index >= MAX_TOKENS) {
                    fprintf(stderr, "Error: Too many tokens\n");
                    return -1;
                }
            }
        }
        // REDIRECT OPERATOR
        /*else if (c=='>') {*/
        /*  char file_descriptor = buffer[buffer_pos-1];*/
        /*  buffer[buffer_pos++] = c;*/
        /*}*/

        else { // Regular character
            buffer[buffer_pos++] = c;
            if (buffer_pos >= MAX_TOKEN_LEN) {
                fprintf(stderr, "Error: Token exceeds maximum length\n");
                return -1;
            }
        }
    }

    // Save the last token if any
    if (buffer_pos > 0) {
        buffer[buffer_pos] = '\0';
        tokens[token_index++] = strdup(buffer);
    }

    // Check for unmatched quotes
    if (in_single_quote || in_double_quote) {
        fprintf(stderr, "Error: Unmatched quote\n");
        return -1;
    }

    *token_count = token_index;
    return 0;
}

void performExpansions(char **tokens, int token_count)
{
  for (int i=0;i<token_count;i++) {

        if (!strcmp(tokens[i], "~")) {
          strcpy(tokens[i], getenv("HOME"));
        }
  }
}

typedef struct {
    char *command;         // Command name
    char *args[MAX_ARGS + 1]; // Arguments + space for NULL
    int argc;              // Argument count
    int redirect_out_fd;   // File descriptor for `>` or `>>`
    char *redirect_out;    // Output redirection file
    int append_out;        // 1 for append (>>), 0 for overwrite (>)
} Command;

int parseCommand(char **tokens, int token_count, Command *cmd) {
    if (token_count == 0) {
        fprintf(stderr, "Error: Empty token list\n");
        return -1;
    }

    memset(cmd, 0, sizeof(Command)); // Initialize the Command structure

    // State 1: Parse command name
    cmd->command = tokens[0];
    cmd->argc = 0;

    for (int i = 0; i < token_count; i++) {
        char *token = tokens[i];

        if (strcmp(token, ">") == 0 || strcmp(token, ">>") == 0 || (isdigit(token[0]) && (strcmp(token + 1, ">") == 0 || strcmp(token + 1, ">>") == 0))) {
            // State 2: Parse redirection operator
            int fd = isdigit(token[0]) ? token[0] - '0' : 1; // Default to stdout
            int append = (strstr(token, ">>") != NULL);

            if (i + 1 >= token_count) {
                fprintf(stderr, "Error: Missing filename after redirection\n");
                return -1;
            }

            cmd->redirect_out_fd = fd;
            cmd->redirect_out = tokens[++i];
            cmd->append_out = append;
        } else {
            // State 3: Parse arguments
            if (cmd->argc >= MAX_ARGS) {
                fprintf(stderr, "Error: Too many arguments\n");
                return -1;
            }
            cmd->args[cmd->argc++] = token;
        }
    }

    return 0; // Success
}

void printCommand(const Command *cmd) {
    printf("Command: %s\n", cmd->command);
    printf("Arguments (%d):\n", cmd->argc);
    for (int i = 0; i < cmd->argc; i++) {
        printf("  [%d]: %s\n", i, cmd->args[i]);
    }

    if (cmd->redirect_out) {
        printf("Redirection:\n");
        printf("  File descriptor: %d\n", cmd->redirect_out_fd);
        printf("  File: %s\n", cmd->redirect_out);
        printf("  Append: %s\n", cmd->append_out ? "Yes" : "No");
    }
}


void executeCommand(Command *cmd) {
    // Handle redirections
    if (cmd->redirect_out) {
        int flags = O_WRONLY | O_CREAT;
        flags |= cmd->append_out ? O_APPEND : O_TRUNC; // Append or overwrite

        int fd = open(cmd->redirect_out, flags, 0644);
        if (fd == -1) {
            perror("Error opening file for redirection");
            exit(EXIT_FAILURE);
        }

        if (dup2(fd, cmd->redirect_out_fd) == -1) {
            perror("Error redirecting output");
            exit(EXIT_FAILURE);
        }

        close(fd); // Close the file descriptor after duplicating
    }

    // Null-terminate the arguments array for execvp
    cmd->args[cmd->argc++] = NULL;

    // Execute the command
    /*printCommand(cmd);*/
    // ECHO COMMAND


    if (strcmp(cmd->command, "echo") == 0) {
        for (int i = 1; i < cmd->argc-1; i++) {
            printf("%s", cmd->args[i]);
            if (i < cmd->argc - 1) {
                printf(" ");
            }
        }
        printf("\n");
        // Flush stdout to ensure output is written immediately
        exit(EXIT_SUCCESS);
        return;
    }

 
    else {
        int status_code = execvp(cmd->command, cmd->args);
        if (status_code == -1) {
            perror("Error executing command");
            exit(EXIT_FAILURE);
        }
    }
}

void quoteStr(char *str)
{
    size_t len = strlen(str);

    // Shift the string right to make space for the opening quote
    for (size_t i = len; i > 0; i--) {
        str[i] = str[i - 1];
    }

    str[0] = '"';
    str[len + 1] = '"';
    str[len + 2] = '\0';
}

void singlequoteStr(char *str)
{
    size_t len = strlen(str);

    // Shift the string right to make space for the opening quote
    for (size_t i = len; i > 0; i--) {
        str[i] = str[i - 1];
    }

    str[0] = '\'';
    str[len + 1] = '\'';
    str[len + 2] = '\0';
}


void expandArgs(char **input, int argc, char *output)
{
    char *out = output;     // Output pointer
    for (int i = 1; i < argc; i++) {
      const char *in = input[i]; // Input pointer
      int counter = 0;
      while (*in) {
          if (*in == '"') { 
              *out++ = '\\'; 
              *out++ = '\"';  
          } else if (*in == '\'') {
              *out++ = '\\'; 
              *out++ = '\'';
          } else if (*in == '\\') {
              *out++ = '\\'; 
              *out++ = '\\';
          } else if (isspace((unsigned char)*in)) {
              *out++ = '\\';
              *out++ = ' ';
          } else {
              *out++ = *in;
          }
          in++;
        }
      *out++ = ' '; 
      }
    *out = '\0';
}
