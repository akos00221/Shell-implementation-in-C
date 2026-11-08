#include "../include/myshell_ctrl_ops.h"
#include "../include/myshell_ast_tree.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

void piping(struct ast_node *head){
  int pipefd[2] = {0};
  if (pipe(pipefd) < 0) exit(1); 
  pid_t id = fork();
  if (id > 0){
    close(pipefd[1]);

    int backup = dup(STDIN_FILENO);
    dup2(pipefd[0], STDIN_FILENO);
    close(pipefd[0]);

    execute_ast_tree(head->right);

    int status;
    waitpid(id, &status, 0);

    dup2(backup, STDIN_FILENO);
    close(backup);
  }
  else if (id == 0){
    close(pipefd[0]);
    dup2(pipefd[1], STDOUT_FILENO);
    close(pipefd[1]);

    execute_ast_tree(head->left);
    exit(0);
  }
  else {
    fprintf(stderr, "Piping failed.\n");
    return;
  }
}
