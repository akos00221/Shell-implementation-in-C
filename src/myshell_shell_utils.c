#include "../include/myshell_shell_utils.h"
#include "../include/shell_builtin/myshell_jobs.h"
#include "../include/shell_builtin/myshell_shell_builtin.h"
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

executables execs = {
  .exec_size = 3000,
  .exec_len = 0,
  .executables_table = NULL
};

char *search_exec(char *argv, int len) {
  // executable key = {.name = argv, .path = ""};
  for (int i = 0; i < execs.exec_len; ++i) {
    if (!strcmp(argv, execs.executables_table[i].name)) {
      return strdup(execs.executables_table[i].path);
    }
  }
  // executable *res = (executable*)bsearch(&key, executables, exec_len,
  // sizeof(executable), compare_exec_name); if (res){
  //	return strdup(res->path);
  // }
  return NULL;
}

void parse_path_exec() {
  execs.executables_table = malloc(execs.exec_size * sizeof(executable));

  const char *env = getenv("PATH");
  if (env == NULL) {
    fprintf(stderr, "No path variables.\n");
    return;
  }
  char *env_cpy = strdup(env);
  if (env_cpy == NULL || execs.executables_table == NULL) {
    fprintf(stderr, "Memory allocation failed.\n");
    return;
  }

  char *token, *saveptr;
  token = strtok_r(env_cpy, ":", &saveptr);

  while (token != NULL) {
    DIR *dir = opendir(token);
    if (dir == NULL) {
      token = strtok_r(NULL, ":", &saveptr);
      continue;
    }
    int dirid = dirfd(dir);
    struct dirent *entry;
    int token_len = strlen(token);
    while ((entry = readdir(dir)) && entry != NULL) {
      if (!faccessat(dirid, entry->d_name, X_OK, 0)) {
        if (execs.exec_len == execs.exec_size) {
          executable *temp =
              realloc(execs.executables_table, 2 * execs.exec_size * sizeof(executable));
          if (temp == NULL) {
            fprintf(stderr, "Memory allocation failed.\n");
            return;
          }
          execs.exec_size *= 2;
          execs.executables_table = temp;
        }
        execs.executables_table[execs.exec_len].path =
            (char *)malloc(token_len + strlen(entry->d_name) + 2);
        if (execs.executables_table[execs.exec_len].path == NULL) {
          fprintf(stderr, "Memory allocation error.\n");
          return;
        }
        execs.executables_table[execs.exec_len].name = strdup(entry->d_name);
        sprintf(execs.executables_table[execs.exec_len].path, "%s/%s", token,
                execs.executables_table[execs.exec_len].name);

        execs.exec_len++;
      }
    }
    closedir(dir);
    token = strtok_r(NULL, ":", &saveptr);
  }
  // qsort(executables, exec_len, sizeof(executable), compare_exec_name);
  free(env_cpy);
}

int search_cmd(char *key) {
  int high = CMD_CNT - 1;
  int low = 0;
  while (low <= high) {
    int mid = low + (high - low) / 2;
    if (!strcmp(commands[mid].name, key))
      return mid;
    else if (strcmp(commands[mid].name, key) < 0)
      low = ++mid;
    else {
      high = --mid;
    }
  }
  return -1;
}
int launch_exec(char **argv, int len) {

  if (strlen(argv[len-1]) == 1 && argv[len-1][0] == '&'){
    //argv[len-1] = NULL;
    jobs(argv, len);
    return 0;
  }

  char *full_path = search_exec(argv[0], strlen(argv[0]));
  
  if (full_path == NULL) {
    // printf("%s\n", full_path);
    return -1;
  }

  pid_t id = fork();
  if (id > 0) {
    int status;
    waitpid(id, &status, 0);
    free(full_path);
    return 0;
  } else if (id == 0) {
    execv(full_path, argv);
  } else {
    free(argv);
    free(full_path);
    fprintf(stderr, "Subprocess failed.");
    exit(1);
  }
  return 1;
}
void launch(char **argv, int argv_len){
  if (argv == NULL || argv[0] == NULL){
    return;
  }
  int ind = search_cmd(argv[0]);
  if (ind == -1 ){
    if (launch_exec(argv, argv_len) == -1){
      printf("%s: not found\n", argv[0]);
    }
    
  }
  else{
    commands[ind].func(argv, argv_len);
  }
}