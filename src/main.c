#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "shell_header.h"

int main(int argc, char *argv[]){
	
    setvbuf(stdout, NULL, _IONBF, 0);
	parse_path_exec();
	rl_completion_entry_function = custom_completion_function;
	initialize_readline();
	//char input[INPUT_BUFFER_SIZE];
	char *input;
	while (1){
		
		input = readline("$ ");
		//fgets(input, INPUT_BUFFER_SIZE-1, stdin);
		//input[strcspn(input, "\n")] = '\0';

		int params = 4;

		int input_tok_len = 0;
		char **input_tok = malloc((params+1)*sizeof(char*));
		if (input_tok == NULL){
			return 1;
		}
		parse_input(input, &input_tok, &params, &input_tok_len);
		free(input);
	
		input_tok[input_tok_len] = NULL;
	
        //for (int i = 0; i < input_tok_len; ++i){
        //    printf("%s %d\n", input_tok[i], strlen(input_tok[i]));
        //}
		//printf("%s\n", command);
		int flag = 0;
		int ind = search_cmd(input_tok[0]);
        for (int i = 0; i < input_tok_len; ++i){
            if (!strcmp(">", input_tok[i]) || !strcmp("1>", input_tok[i])){
                input_tok[i] = NULL;
                int fd = open(input_tok[i+1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd == -1){
                    //bruh
                }
                int backup = dup(STDOUT_FILENO);
                dup2(fd, STDOUT_FILENO);
                close(fd);
				// run prog
				if (ind == -1){
					if (launch_exec(input_tok, input_tok_len) == -1){
						printf("%s: not found\n", input_tok[0]);
					}

				}
				else{
					commands[ind].func(input_tok, input_tok_len);
				}
				for (int i = 0; i < input_tok_len; ++i){
					free(input_tok[i]);
				}
                dup2(backup, STDOUT_FILENO);
                close(backup);
				flag = 1;
            }
            else if (!strcmp("2>", input_tok[i])){
                input_tok[i] = NULL;
                int fd = open(input_tok[i+1],  O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd == -1){
                    //bruh
                }
                int backup = dup(STDERR_FILENO);
                dup2(fd, STDERR_FILENO);
                close(fd);
				// run prog
				if (ind == -1){
					if (launch_exec(input_tok, input_tok_len) == -1){
						printf("%s: not found\n", input_tok[0]);
					}

				}
				else{
					commands[ind].func(input_tok, input_tok_len);
				}
				for (int i = 0; i < input_tok_len; ++i){
					free(input_tok[i]);
				}
                dup2(backup, STDERR_FILENO);
                close(backup);
				flag = 1;
            }
            else if (!strcmp(">>", input_tok[i]) || !strcmp("1>>", input_tok[i])){
                input_tok[i] = NULL;
				int fd = open(input_tok[i+1], O_APPEND | O_CREAT | O_WRONLY, 0644);
                if (fd == -1){
                    //bruh
                }
                int backup = dup(STDOUT_FILENO);
                dup2(fd, STDOUT_FILENO);
                close(fd);
				// run prog
				if (ind == -1){
					if (launch_exec(input_tok, input_tok_len) == -1){
						printf("%s: not found\n", input_tok[0]);
					}

				}
				else{
					commands[ind].func(input_tok, input_tok_len);
				}
				for (int i = 0; i < input_tok_len; ++i){
					free(input_tok[i]);
				}
                dup2(backup, STDOUT_FILENO);
                close(backup);
				flag = 1;
            }
            else if (!strcmp("2>>", input_tok[i])){
                input_tok[i] = NULL;
                int fd = open(input_tok[i+1],  O_APPEND | O_CREAT | O_WRONLY, 0644);
                if (fd == -1){
                    //bruh
                }
                int backup = dup(STDERR_FILENO);
                dup2(fd, STDERR_FILENO);
                close(fd);
				// run prog
				if (ind == -1){
					if (launch_exec(input_tok, input_tok_len) == -1){
						printf("%s: not found\n", input_tok[0]);
					}

				}
				else{
					commands[ind].func(input_tok, input_tok_len);
				}
				for (int i = 0; i < input_tok_len; ++i){
					free(input_tok[i]);
				}
                dup2(backup, STDERR_FILENO);
                close(backup);
				flag = 1;
            }
        }
		if (flag == 0){
			if (ind == -1){
				if (launch_exec(input_tok, input_tok_len) == -1){
					printf("%s: not found\n", input_tok[0]);
				}

			}
			else{
				commands[ind].func(input_tok, input_tok_len);
			}
			for (int i = 0; i < input_tok_len; ++i){
				free(input_tok[i]);
			}
		}

		free(input_tok);
		
	}
}

/*
		char *inp_ptr = input;
		while (*inp_ptr == ' ') {inp_ptr++;}
		char command[50];
		int j = 0;
		while (j < strcspn(inp_ptr, " ")) {
			command[j] = inp_ptr[j];
			j++;
		}
		command[j] = '\0';

*/