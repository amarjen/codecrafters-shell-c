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

#define MAX_TOKENS 100 // Maximum number of tokens
#define MAX_TOKEN_LEN 256 // Maximum token length

int parse_tokens(const char *input, char **tokens, int *token_count) {
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
             /*buffer[buffer_pos++] = c;*/
             buffer[buffer_pos++] = '\\';
             buffer[buffer_pos++] = next_c;
             /**input++;*/
           }
           else {
             *input--;
             buffer[buffer_pos++] = c;
           /*is_escaped = 1;*/
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

void quoteStr(char *str) {
    size_t len = strlen(str);

    // Shift the string right to make space for the opening quote
    for (size_t i = len; i > 0; i--) {
        str[i] = str[i - 1];
    }

    str[0] = '"';
    str[len + 1] = '"';
    str[len + 2] = '\0';
}
void expandArgs(char **input, int argc, char *output) {
    char *out = output;     // Output pointer
    // Para todo el texto
    /**out++ = '"';*/

    for (int i = 1; i < argc; i++) {
      // Por cada token
      const char *in = input[i]; // Input pointer
      /**out++ = '\\';*/
      /**out++ = '\'';*/
      int counter = 0;
      // Process each character in the input string
      while (*in) {
          // Por cada caracter
          /*printf("%d: %c\n", ++counter, *in);*/
          if (*in == '"') { // Escape double quotes
              *out++ = '\\'; // Add the backslash
              *out++ = '\"';  // Add the escaped double quote
          } else if (*in == '\'') {
              *out++ = '\\'; // Add the backslash
              *out++ = '\''; // Add the backslash
          } else if (*in == '\\') {
              *out++ = '\\'; // Add the backslash
              *out++ = '\\'; // Add the backslash
          } else if (isspace((unsigned char)*in)) { // Handle spaces as argument separators
              // Add a closing and opening double quote for separated arguments
              *out++ = '\\';
              *out++ = ' ';
          } else {
              *out++ = *in;  // Copy the character as-is
          }
          in++;
        }
      /**out++ = '\\';*/
      /**out++ = '\''; */
      *out++ = ' '; 

    /**out++ = '"';*/
      }

    /**out++ = '"';*/
    *out = '\0';

    // Print the result
    /*printf("%s\n", args_quotes);*/
}

