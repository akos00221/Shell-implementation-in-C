#include "../include/myshell_redir.h"
#include "../include/myshell_shell_utils.h"
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

static void redirect(struct ast_node *head, const int fd_, unsigned int oflag, int permissions){
  int fd = open(head->right->exec[0], oflag, permissions);
  if (fd == -1) { 
    fprintf(stderr, "Creating/Opening \"%s\" failed\n", head->right->exec[0]);
  }
  int backup = dup(fd_);
  dup2(fd, fd_);
  close(fd);
  launch(head->left->exec, head->left->exec_len);
  dup2(backup, fd_);
  close(backup);
}
// > 1>
void redirect_stdout_w(struct ast_node *head){
  redirect(head, STDOUT_FILENO, (O_WRONLY | O_CREAT | O_TRUNC), 0644);
}
// 2>
void redirect_stderr_w(struct ast_node *head){
  redirect(head, STDERR_FILENO, (O_WRONLY | O_CREAT | O_TRUNC), 0644);
}
// >> 1>>
void redirect_stdout_a(struct ast_node *head){
  redirect(head, STDOUT_FILENO, (O_APPEND | O_CREAT | O_WRONLY), 0644);
}
// 2>> 
void redirect_stderr_a(struct ast_node *head){
  redirect(head, STDERR_FILENO, (O_APPEND | O_CREAT | O_WRONLY), 0644);
}