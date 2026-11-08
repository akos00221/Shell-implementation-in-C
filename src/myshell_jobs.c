#include "../include/shell_builtin/myshell_jobs.h"
#include "../include/myshell_shell_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

const char *job_states_lookup[NUM_OF_JOB_STATES] = {
  [RUNNING] = "Running", [STOPPED] = "Stopped", [DONE] = "Done"
};

jobs_list jl = {
  .jobs_list_size = 8,
  .jobs_list_len = 0,
  .next_job_num = 1,
  .data = NULL 
}; 

void mark_jobs(){
  for (int i = 0; i < jl.jobs_list_len; ++i){

    job *cur = &jl.data[i];
    int status;
    pid_t waitpid_ret = waitpid(cur->id, &status, WNOHANG);
      
    if (waitpid_ret == cur->id && WIFEXITED(status)){
      cur->state = DONE;
    }
  }
}
void print_jobs(print_style ps){
  switch (ps) {
    case PRINT_DONE:
      for (int i = 0; i < jl.jobs_list_len; ++i){
        job *cur = &(jl.data[i]);
        if (jl.data[i].state == DONE){
          printf("[%d]%s  %-*s", cur->job_num, (i == jl.jobs_list_len-2) ? "-" : (i == jl.jobs_list_len-1) ? "+" : "", 24, job_states_lookup[cur->state]);
          for (int j = 0; j < cur->cmd_len-1; ++j){
            printf("%s ", cur->cmd[j]);
          }
          printf("%s\n", cur->cmd[cur->cmd_len-1]);
        }
      }
      break;
    default:
        for (int i = 0; i < jl.jobs_list_len; ++i){
        job *cur = &(jl.data[i]);
        printf("[%d]%s  %-*s", cur->job_num, (i == jl.jobs_list_len-2) ? "-" : (i == jl.jobs_list_len-1) ? "+" : "", 24, job_states_lookup[cur->state]);
        for (int j = 0; j < cur->cmd_len-1; ++j){
          printf("%s ", cur->cmd[j]);
        }
        printf("%s\n", cur->cmd[cur->cmd_len-1]);
      }
      break;
  }
}
void sweep_jobs(){
    for (int i = 0; i < jl.jobs_list_len; ++i){
      job *cur = &(jl.data[i]);
      if (cur->state == DONE){
        if (cur->job_num < jl.next_job_num){
          jl.next_job_num = cur->job_num;
        }
        for (int j = 0; j < cur->cmd_len; ++j) free(cur->cmd[j]);
        //free(cur->cmd);
        for (int j = i; j < jl.jobs_list_len-1; j++){
          jl.data[j] = jl.data[j+1];
        }
        jl.jobs_list_len--;
        i--;
      }
    }
}
void jobs(char **argv, int len) {
  
  if (!strcmp(argv[0], "jobs")){
    if (jl.jobs_list_len == 0){
      return;
    }
    mark_jobs();
    print_jobs(PRINT_ALL);
    sweep_jobs();
    return;
  }

  char *full_path = search_exec(argv[0], strlen(argv[0]));
  pid_t id = fork();
  argv[len-1] = NULL;
  int len_wo_and = len-1;

  if (full_path == NULL){
    return;
  }
  if (id > 0){
    if (jl.jobs_list_len >= jl.jobs_list_size || jl.data == NULL){
      job *temp = (job *)realloc(jl.data, 2*jl.jobs_list_size*sizeof(job));
      if (temp == NULL){
        fprintf(stderr, "Memory allocation failed.\n");
        exit(1);
      }
      jl.jobs_list_size *= 2;
      jl.data = temp;
    }
    jl.data[jl.jobs_list_len].job_num = jl.next_job_num;
    jl.data[jl.jobs_list_len].state = RUNNING;
    jl.data[jl.jobs_list_len].id = id;
    jl.data[jl.jobs_list_len].cmd = malloc(len*sizeof(char *));
    if (jl.data[jl.jobs_list_len].cmd == NULL) {}
    for (int i = 0; i < len_wo_and; ++i) jl.data[jl.jobs_list_len].cmd[i] = strdup(argv[i]);
    jl.data[jl.jobs_list_len].cmd_len = len_wo_and;
    printf("[%d] %d\n", jl.data[jl.jobs_list_len].job_num, id);
    jl.jobs_list_len++;
    jl.next_job_num = jl.jobs_list_len+1;
  }
  else if (id == 0){
    //printf("%s\n", full_path);
    execv(full_path, argv);
  }
  else if (id < 0){
    fprintf(stderr, "Subprocess failed.");
    exit(1);
  }
  
}