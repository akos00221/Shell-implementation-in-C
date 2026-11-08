#include "../include/shell_builtin/myshell_shell_builtin.h"
#include <stdio.h>
#include <readline/readline.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <wordexp.h>

#include "../include/shell_builtin/myshell_jobs.h"
#include "../include/shell_builtin/myshell_history.h"
#include "../include/shell_builtin/myshell_declare.h"

const builtin_command commands[CMD_CNT] = {{&(cd), "cd"},
                                           {&(complete), "complete"},
                                           {&(declare), "declare"},
                                           {&(echo), "echo"},
                                           {&(exit_shell), "exit"},
                                           {&(history), "history"},
                                           {&(jobs), "jobs"},
                                           {&(pwd), "pwd"},
                                           {&(type), "type"}
                                           

};

custom_completions cstm_cmpls = {
  .completions_len = 0,
  .completions_size = 8, // the actual array will be initialized to sizeof(executable)*16
  .custom_completions_table = NULL
};

void complete(char **argv, int len) {

  switch (argv[1][1]) {
    case 'p':
      for (int i = 0; i < cstm_cmpls.completions_len; ++i) {
        if (!strcmp(cstm_cmpls.custom_completions_table[i].name, argv[2])) {
          printf("complete -C '%s' %s\n", cstm_cmpls.custom_completions_table[i].path,
                cstm_cmpls.custom_completions_table[i].name);
          return;
        }
      }
      printf("complete: %s: no completion specification\n", argv[2]);
      break;
    case 'C':
      // printf("%s %s %s %s\n", argv[0], argv[1], argv[2], argv[3]);
      if (cstm_cmpls.completions_len >= cstm_cmpls.completions_size || cstm_cmpls.custom_completions_table == NULL) {
        executable *temp = realloc(cstm_cmpls.custom_completions_table,
                                  cstm_cmpls.completions_size * 2 * sizeof(executable));
        if (temp == NULL) {
          fprintf(stderr, "Failed to allocate memory.\n");
          return;
        }
        cstm_cmpls.custom_completions_table = temp;
        cstm_cmpls.completions_size *= 2;
      }
      cstm_cmpls.custom_completions_table[cstm_cmpls.completions_len].path = strdup(argv[2]);
      cstm_cmpls.custom_completions_table[cstm_cmpls.completions_len++].name = strdup(argv[3]);
      break;
    case 'r':
      for (int i = 0; i < cstm_cmpls.completions_len; ++i){
        if (!strcmp(cstm_cmpls.custom_completions_table[i].name, argv[2])) {
          free(cstm_cmpls.custom_completions_table[i].name);
          free(cstm_cmpls.custom_completions_table[i].path);
          cstm_cmpls.custom_completions_table[i] = cstm_cmpls.custom_completions_table[cstm_cmpls.completions_len-1];
          cstm_cmpls.completions_len--;
        }
        
      }
      break;
  }

}

void type(char **argv, int len) {

  int ind = search_cmd(argv[1]);
  if (ind == -1) {
    char *full_path = search_exec(argv[1], strlen(argv[1]));
    if (full_path != NULL) {
      printf("%s is %s\n", argv[1], full_path);
      free(full_path);
    } else {
      printf("%s: not found\n", argv[1]);
    }
  } else {
    printf("%s is a shell builtin\n", argv[1]);
  }
}
void echo(char **argv, int len) {
  for (int i = 1; i < len-1; ++i) {
    printf("%s ", argv[i]);
  }
  printf("%s", argv[len-1]);
  printf("\n");
}
void exit_shell(char **argv, int len) {

  for (int i = 0; i < cstm_cmpls.completions_len; ++i) {
    free(cstm_cmpls.custom_completions_table[i].name);
    free(cstm_cmpls.custom_completions_table[i].path);
  }
  free(cstm_cmpls.custom_completions_table);

  for (int i = 0; i < execs.exec_len; ++i) {
    free(execs.executables_table[i].name);
    free(execs.executables_table[i].path);
  }
  free(execs.executables_table);

  for (int i = 0; i < jl.jobs_list_len; ++i){
    for (int j = 0; j < jl.data[i].cmd_len; j++) {free(jl.data[i].cmd[j]);}
    free(jl.data[i].cmd);
  }
  free(jl.data);

  for (int i = 0; i < len; ++i)
    free(argv[i]);
  free(argv);

  for (int i = 0; i < vars.len; i++){
    free(vars.data[i].name);
    free(vars.data[i].val);
  }
  free(vars.data);
  
  save_hist_to_file();

  exit(EXIT_SUCCESS);
}
void pwd(char **argv, int len){
  char *dir = getcwd(NULL, 0);
  if (!dir){
    fprintf(stderr, "Cannot access current working directory.\n");
    return;
  }
  printf("%s\n", dir);
  free(dir);
}
void cd(char **argv, int len){
  if (len < 2){
    return;
  }
  wordexp_t expanded_arg;
  if (wordexp(argv[1], &expanded_arg, 0)){
    fprintf(stderr, "Path expansion failed\n.");
    return;
  }
  if (chdir(expanded_arg.we_wordv[0])){
    printf("cd: %s: No such file or directory\n", argv[1]);
  }
  wordfree(&expanded_arg);
}