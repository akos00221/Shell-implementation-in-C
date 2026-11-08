#ifndef MYSHELL_AST_TREE_H
#define MYSHELL_AST_TREE_H

#define NUM_OF_REDIR_OPERATORS 6
#define NUM_OF_CTRL_OPERATORS 4

struct ast_node {
	struct ast_node *left;
	struct ast_node *right;
	char op[4];
	unsigned int exec_len;
	char **exec;
};

typedef void (*op_lookup_func)(struct ast_node*);

typedef struct {
  const char operator_repr[4];
  int val;
  op_lookup_func func;
}operator_struct;

struct op_lookup_struct {
  int highest_precedence;
  int table_len;
  operator_struct table[];
};

struct ast_node *create_ast_node();
void create_ast_tree(char **input_tok, int input_tok_len, struct ast_node *head);
void print_ast(struct ast_node *head);
void execute_ast_tree(struct ast_node *head);
void free_ast_tree(struct ast_node *head);

#endif 