#ifndef MYSHELL_INPUT_PARSE_H
#define MYSHELL_INPUT_PARSE_H

#define INPUT_BUFFER_SIZE 4096

typedef enum {
	NORMAL,
	NORMAL_ESC,
	SINGLE_QUOTE,
	DOUBLE_QUOTE,
	DOUBLE_QUOTE_ESC
}parse_states;

void parse_input(char *input, char ***input_tok, int *size, int *len);

#endif