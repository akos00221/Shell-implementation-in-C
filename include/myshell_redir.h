#ifndef MYSHELL_REDIR_H
#define MYSHELL_REDIR_H

#include "../include/myshell_ast_tree.h"


void redirect_stdout_w(struct ast_node *head);
void redirect_stderr_w(struct ast_node *head);
void redirect_stdout_a(struct ast_node *head);
void redirect_stderr_a(struct ast_node *head);

#endif