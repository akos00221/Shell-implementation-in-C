
#include <stdlib.h>
#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "../include/myshell_shell_utils.h"
#include "../include/myshell_ast_tree.h"
#include "../include/shell_builtin/myshell_jobs.h"
#include "../include/myshell_input_parse.h"
#include "../include/myshell_readline_util.h"
#include "../include/shell_builtin/myshell_history.h"

int main(int argc, char *argv[]){
	
    setvbuf(stdout, NULL, _IONBF, 0);
	parse_path_exec();
	rl_completion_entry_function = custom_completion_function;
	initialize_readline();
	initalize_history();
	char *input;
	while (1){
		
		input = readline("$ ");
		int params = 4;

		int input_tok_len = 0;
		char **input_tok = malloc((params)*sizeof(char*));
		if (input_tok == NULL){
			return 1;
		}
		add_history(input);

		parse_input(input, &input_tok, &params, &input_tok_len);
		free(input);

		struct ast_node *head = create_ast_node();
		create_ast_tree(input_tok, input_tok_len, head);
		//print_ast(head);
		execute_ast_tree(head);
		for (int i = 0; i < input_tok_len; i++) free(input_tok[i]);
		free(input_tok);
		free_ast_tree(head);

		mark_jobs();
		print_jobs(PRINT_DONE);
		sweep_jobs();
	}
}
