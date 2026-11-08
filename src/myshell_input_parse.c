#include "../include/myshell_input_parse.h"
#include "../include/shell_builtin/myshell_declare.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const unsigned char char_lookup[256] = {
    [' '] = 1,  ['\t'] = 1, ['\n'] = 1, ['\\'] = 2,
    ['\"'] = 2, ['$'] = 2,  ['`'] = 2};

static void create_token(char ***input_tok, int *size, int *len,
                         char token_buffer[], int *token_buffer_len) {
  
  token_buffer[*token_buffer_len] = '\0';
  //printf("%s %d\n", token_buffer, *token_buffer_len);
  if (*len >= *size) {
    char **temp = realloc(*input_tok, (*size) * 2 * sizeof(char *));
    if (temp == NULL) {
      fprintf(stderr, "Memory allocation error.\n");
      return;
    }
    (*size) *= 2;
    (*input_tok) = temp;
  }
  (*input_tok)[*len] = strdup(token_buffer);
  *token_buffer_len = 0;
  (*len)++;
}

void parse_input(char *input, char ***input_tok, int *size, int *len) {
  if (!input){
    return;
  }
  char c;
  char token_buffer[INPUT_BUFFER_SIZE] = {0};
  parse_states state = NORMAL;
  int j = 0, token_buffer_len = 0;
  *len = 0;
  while ((c = input[j]) != '\0') {

    switch (state) {
    case NORMAL:

      if (char_lookup[(unsigned char)c] & 1) {
        if (token_buffer_len != 0) {
          create_token(input_tok, size, len, token_buffer, &token_buffer_len);
        }
      } else if (c == '\\') {
        state = NORMAL_ESC;
      } else if (c == '\'') {
        state = SINGLE_QUOTE;
      } else if (c == '\"') {
        state = DOUBLE_QUOTE;
      } else if (c == '$'){
        j++;
        expand_var(input, &token_buffer, &token_buffer_len, &j);
      } else {
        token_buffer[token_buffer_len++] = c;
      }

      break;
    case NORMAL_ESC:
      token_buffer[token_buffer_len++] = c;
      state = NORMAL;
      break;
    case SINGLE_QUOTE:
      if (c == '\'') {
        if (input[j + 1] == '\'' || input[j - 1] == '\'') {
          state = NORMAL;
        } else {
          create_token(input_tok, size, len, token_buffer, &token_buffer_len);
          state = NORMAL;
        }
      } else {
        token_buffer[token_buffer_len] = c;
        token_buffer_len++;
      }
      break;
    case DOUBLE_QUOTE:

      if (c == '\"') {
        if (input[j + 1] == '\"' || input[j - 1] == '\"' ||
            !isspace(input[j + 1])) {
          state = NORMAL;
        } else {
          create_token(input_tok, size, len, token_buffer, &token_buffer_len);
          state = NORMAL;
        }
      } else if (c == '\\') {
        state = DOUBLE_QUOTE_ESC;
      } else {
        token_buffer[token_buffer_len++] = c;
      }

      break;
    case DOUBLE_QUOTE_ESC:
      if (char_lookup[(unsigned char)c] > 0) {
        token_buffer[token_buffer_len++] = c;
      }
      state = DOUBLE_QUOTE;
      break;
    }

    j++;
  }
  if (token_buffer_len > 0) {
    create_token(input_tok, size, len, token_buffer, &token_buffer_len);
  }
}