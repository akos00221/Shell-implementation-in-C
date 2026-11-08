#ifndef MYSHELL_SHELL_BUILTIN_H
#define MYSHELL_SHELL_BUILTIN_H

#define CMD_CNT 9

#include "../myshell_shell_utils.h"

//static int completions_size = 16 / 2, completions_len = 0;
//static executable *custom_completions = NULL;

typedef struct {
	size_t completions_size, completions_len;
	executable *custom_completions_table;
}custom_completions;

extern custom_completions cstm_cmpls;

void exit_shell(char **argv, int len);
void echo(char **argv, int len);
void type(char **argv, int len);
void complete(char **argv, int len);
void pwd(char **argv, int len);
void cd(char **argv, int len);

typedef void (*command_function)(char**, int);

typedef struct {
	command_function func;
	char *name;
}builtin_command;

extern const builtin_command commands[CMD_CNT];


#endif