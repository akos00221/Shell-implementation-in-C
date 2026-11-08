#include <stdio.h>
#include <readline/readline.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include "../include/myshell_readline_util.h"
#include "../include/shell_builtin/myshell_shell_builtin.h"
#include "../include/myshell_shell_utils.h"

#define ENV_VAR_LEN 3

static char *command_completion_function(const char *text, int state) {
  static int list_index = 0, len = 0, stage = 0;
  char *name;

  /* If this is a new word to complete, initialize now.  This includes
          saving the length of TEXT for efficiency, and initializing the index
          variable to 0. */
  if (!state) {
    list_index = 0;
    len = strlen(text);
  }

  /* Return the next name which partially matches from the command list. */
  if (stage == 0) {
    while (list_index < CMD_CNT) {
      name = commands[list_index].name;
      list_index++;

      if (strncmp(name, text, len) == 0)
        return (strdup(name));
    }
    stage = 1;
    list_index = 0;
  }
  if (stage == 1) {
    while (list_index < execs.exec_len) {
      name = execs.executables_table[list_index].name;
      list_index++;

      if (strncmp(name, text, len) == 0)
        return (strdup(name));
    }
  }

  stage = 0;

  /* If no names matched, then return NULL. */
  return ((char *)NULL);
}
char *custom_completion_function(const char *text, int state) {
  static int txt_len = 0, output_size = 16, output_len = 0, match_ind = 0;
  static char **output = NULL;

  if (!state) {
    int line_tok_size = 8, line_tok_len = 0, line_len;
    char *save_ptr;
    if (!rl_line_buffer || rl_line_buffer[0] == '\0') {
      return NULL;
    }

    char *line = strdup(rl_line_buffer);
    if (!line) {
      return NULL;
    }
    line_len = strlen(line);

    char **env_var = (char **)malloc(ENV_VAR_LEN*sizeof(char *));
            
    asprintf(&env_var[0], "COMP_LINE=%s", rl_line_buffer);
    asprintf(&env_var[1], "COMP_POINT=%d", line_len);
    env_var[2] = NULL;

    char **line_tok = (char **)malloc(line_tok_size*sizeof(char*));
    if (!line_tok) {
      fprintf(stderr, "Memory allocation for custom completion variable failed.\n");
      return NULL;
    }
    
    //line_tok[line_tok_len++] = NULL;
    line_tok[line_tok_len++] = strtok_r(line, " \t", &save_ptr);
    //line_tok[line_tok_len++] = strdup(text);
    

    // path, cmd, cur, cur-1, null
    char *args[5];

    while ((line_tok[line_tok_len] = strtok_r(NULL, " \t", &save_ptr)) != NULL){
      line_tok_len++;
      if (line_tok_len > line_tok_size){
        char **temp = realloc(line_tok, line_tok_size * 2 * sizeof(char*));
        if (!temp) { exit(1); }
        line_tok_size *= 2;
        line_tok = temp;
      }
    }
    args[1] = line_tok[0];
    args[2] = strdup(text);
    if (line_tok_len > 1){
      args[3] = line_tok[line_tok_len-2];
    }
    else {
      args[3] = NULL;
    }
    
    args[4] = NULL;
    
    //line_tok_len--;
    //line_tok[line_tok_len] = NULL;
    //for (int i = 0; i < line_tok_len; ++i) printf("%s ", line_tok[i]);
    match_ind = 0;
    txt_len = strlen(text);
    int cmd_len = strlen(args[1]);
    for (int i = 0; i < cstm_cmpls.completions_len; ++i) {
      if (!strncmp(args[1], cstm_cmpls.custom_completions_table[i].name, cmd_len)) {
        int pipefd[2] = {0};

        if (pipe(pipefd) < 0)
          break;
        pid_t id = fork();

        if (id > 0) {
          close(pipefd[1]);
          output = malloc(output_size * sizeof(char *));
          if (!output){ exit(1); }
          for (int i = 0; i < output_size; i++)
            output[i] = NULL;
          size_t s = 0;
          int getline_ind;
          FILE *output_ptr = fdopen(pipefd[0], "r");
          while ((getline_ind = getline(&output[output_len], &s, output_ptr)) &&
                 getline_ind != -1) {
            output[output_len++][getline_ind - 1] = '\0';
            if (output_len >= output_size){
              char **temp = realloc(output, output_size * sizeof(char *) * 2);
              if (!temp){
                exit(1);
              }
              output_size *= 2;
              output = temp;
            }
            s = 0;
          }
          int status;
          waitpid(id, &status, 0);
          fclose(output_ptr);
          break;
        } 
        else if (id == 0) {
          	close(pipefd[0]);
          	dup2(pipefd[1], STDOUT_FILENO);
          	close(pipefd[1]);
            //line_tok[0] = custom_completions[i].path;
            args[0] = cstm_cmpls.custom_completions_table[i].path;
            execve(cstm_cmpls.custom_completions_table[i].path, args, env_var);
        } 
        else {
          fprintf(stderr, "Subprocess failed.\n");
          return NULL;
        }
      }
    }
    free(env_var[0]);
    free(env_var[1]);
    free(env_var);
    //free(line_tok[2]);
    free(line_tok);
    free(line);
  }

  for (; match_ind < output_len; match_ind++) {
    
    char *current = output[match_ind];
    //printf("%s\n", current);
    if (!strncmp(text, current, txt_len)) {
      match_ind++;
      return strdup(current);
    }
  }

  for (int i = 0; i < output_len; ++i)
    free(output[i]);
  output_len = 0;
  free(output);
  return ((char *)NULL);
}

void initialize_readline(void) {
  /* Allow conditional parsing of the ~/.inputrc file. */
  rl_readline_name = "myshell";

  /* Tell the completer that we want a crack first. */
  rl_attempted_completion_function = myshell_completion;
}

char **myshell_completion(const char *text, int start, int end) {
  rl_attempted_completion_over = 1;
  /* If this word is at the start of the line, then it is a command
     to complete.  Otherwise it is the name of a file in the current
     directory. */
  rl_filename_completion_desired = 0;
  if (start == 0) {
    return rl_completion_matches(text, command_completion_function);
  }
  char *cmd = strndup(rl_line_buffer, strcspn(rl_line_buffer, " \t"));
  int cmd_len = strlen(cmd);
  int has_custom = 0;

  for (int i = 0; i < cstm_cmpls.completions_len; ++i) {
    if (!strncmp(cmd, cstm_cmpls.custom_completions_table[i].name, cmd_len)) {
      has_custom = 1;
      break;
    }
  }
  free(cmd);

  if (has_custom) {
    char **ret_val = rl_completion_matches(text, custom_completion_function);
    
    return ret_val;
  }

  rl_filename_completion_desired = 1;
  return rl_completion_matches(text, rl_filename_completion_function);
}