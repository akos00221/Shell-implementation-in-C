#ifndef SHELL_H
#define SHELL_H

#define CMD_CNT 4
#define INPUT_BUFFER_SIZE 4096
#define ESCAPE_CHARS_LEN 4
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <readline/readline.h>
#include <readline/history.h>
//extern const char double_quotes_escape_chars[ESCAPE_CHARS_LEN];

typedef enum {
	NORMAL,
	NORMAL_ESC,
	SINGLE_QUOTE,
	DOUBLE_QUOTE,
	DOUBLE_QUOTE_ESC
}parse_states;

typedef void (*command_function)(char**, int);
typedef struct {
	command_function func;
	char *name;
}builtin_command;

typedef struct {
	char *name, *path;
}executable;


//readline util
char *custom_completion_function(const char *text, int state);
void initialize_readline(void);
char **myshell_completion(const char *text, int start, int end);

// shell builtin
void exit_shell(char **argv, int len);
void echo(char **argv, int len);
void type(char **argv, int len);
void redirect_stdout(char **argv, int len);
void complete(char **argv, int len);
// shell util
int search_cmd(char *key);
int launch_exec(char **argv, int len);
void parse_input(char *input, char ***input_tok, int *size, int *len);
void parse_path_exec();

extern const builtin_command commands[CMD_CNT];

#endif