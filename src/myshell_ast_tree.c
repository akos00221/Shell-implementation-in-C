#include "../include/myshell_ast_tree.h"
#include "../include/myshell_redir.h"
#include "../include/myshell_shell_utils.h"
#include "../include/myshell_ctrl_ops.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static struct op_lookup_struct op_redir = {
  .table = {
    {">", 2, &redirect_stdout_w},
    {"1>", 2, &redirect_stdout_w},
    {"2>", 2, &redirect_stderr_w},
    {">>", 2, &redirect_stdout_a},
    {"1>>", 2, &redirect_stdout_a},
    {"2>>", 2, &redirect_stderr_a}
  },
  .table_len = NUM_OF_REDIR_OPERATORS,
  .highest_precedence = 2
};

static struct op_lookup_struct op_ctrl = {
  .table = {
    {";", 0},
    {"&&", 1},
    {"||", 1},
    {"|", 2, &piping}
  },
  .table_len = NUM_OF_CTRL_OPERATORS,
  .highest_precedence = 2
};


static void print_ast_helper(struct ast_node *head, int depth) {
  if (head == NULL) {
    return;
  }

  // 1. Visit Right subtree first (pushes the right branch to the top of the terminal)
  print_ast_helper(head->right, depth + 1);

  // 2. Print the current node
  // Print leading spaces based on how deep we are in the tree
  for (int i = 0; i < depth; i++) {
    printf("        "); 
  }
  
  // Print the execution array on a single line so it acts as one "node" visually
  printf("├── [ ");
  for (int i = 0; i < head->exec_len; i++) {
    printf("%s ", head->exec[i]);
  }
  printf("]\n");

  // 3. Visit Left subtree (pushes the left branch to the bottom of the terminal)
  print_ast_helper(head->left, depth + 1);
}

// Main function wrapper
void print_ast(struct ast_node *head) {
  printf("\n--- AST Visual ---\n");
  print_ast_helper(head, 0);
  printf("------------------\n\n");
}

struct ast_node *create_ast_node(){
  struct ast_node *new_node = (struct ast_node*)malloc(sizeof(struct ast_node));
  new_node->left = NULL;
  new_node->right = NULL;
  new_node->exec = NULL;
  new_node->exec_len = 0;
  new_node->op[0] = '\0';
  return new_node;
  
} 
void create_ast_tree(char **input_tok, int input_tok_len, struct ast_node *head){
  if (input_tok == NULL){
    return;
  }
  int ind = -1, redir_ind = -1, min_priority = op_ctrl.highest_precedence+1;
  for (int i = 0; i < input_tok_len; ++i){
    for (int h = 0; h < op_ctrl.table_len; ++h){
      if (!strcmp(op_ctrl.table[h].operator_repr, input_tok[i]) && op_ctrl.table[h].val <= min_priority){
        min_priority = op_ctrl.table[h].val;
        ind = i;
      }
    }
  }
  if (ind == -1){
    for (int j = input_tok_len-1; j >= 0; j--){
      for (int i = 0; i < op_redir.table_len; ++i){
        if (!strcmp(input_tok[j], op_redir.table[i].operator_repr)){
          
          strncpy(head->op, input_tok[j], 4);
          head->right = create_ast_node();
          head->right->exec = (char **)malloc(sizeof(char *));
          //printf("%s\n", input_tok[j+1]);
          head->right->exec[0] = input_tok[j+1];
          head->right->exec_len = 1;

          head->left = create_ast_node();
          head->left->exec = (char **)malloc((input_tok_len-1)*sizeof(char *));
          head->left->exec_len = input_tok_len-2;
          int h;
          for (h = 0; h < j; h++){
            head->left->exec[h] = input_tok[h];
          }
          int k = h;
          for (int h = j+2; h < input_tok_len; h++){
            head->left->exec[k] = input_tok[h];
            k++;
          }
          head->left->exec[k] = NULL;
          return;
        }
      }
    }
    // +1 for NULL end of arg list
    head->exec = (char **)malloc((input_tok_len+1)*sizeof(char *));
    for (int i = 0; i < input_tok_len; ++i){
      head->exec[i] = input_tok[i];
    }
    head->exec[input_tok_len] = NULL;
    head->exec_len = input_tok_len;
  }
  else{
    strncpy(head->op, input_tok[ind], 4);
    head->left = create_ast_node();
    create_ast_tree(input_tok, ind, head->left);
    head->right = create_ast_node();
    create_ast_tree((input_tok+ind+1), (input_tok_len-ind-1), head->right);
    
  }
}


void execute_ast_tree(struct ast_node *head){
  if (head == NULL){
    return;
  }
  if (head->op[0] == '\0'){
    launch(head->exec, head->exec_len);
  }
  else{
    for (int i = 0; i < NUM_OF_CTRL_OPERATORS; i++){
      if (!strcmp(head->op, op_ctrl.table[i].operator_repr)){
        op_ctrl.table[i].func(head);
        return;
      }
    }
    for (int i = 0; i < NUM_OF_REDIR_OPERATORS; i++){
      if (!strcmp(head->op, op_redir.table[i].operator_repr)){
        op_redir.table[i].func(head);
        return;
      }
    }
  }
}
void free_ast_tree(struct ast_node *head){
  if (head == NULL){
    return;
  }
  free_ast_tree(head->left);
  free_ast_tree(head->right);
  if (head->exec != NULL){
    free(head->exec);
  }
  free(head);
}