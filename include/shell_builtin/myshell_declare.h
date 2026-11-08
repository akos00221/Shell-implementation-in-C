#ifndef MYSHELL_SHELL_BUILTIN_DECLARE_H
#define MYSHELL_SHELL_BUILTIN_DECLARE_H

#include <stddef.h>

typedef struct {
    char *name, *val;
    size_t name_len, val_len;
}variable;

typedef struct {
    size_t len, size;
    variable *data;
}variable_list;

extern variable_list vars;

void declare(char **argv, int len);
void expand_var(char *argv, char (*token_buffer)[4096], int *token_buffer_len, int *j);
//void expand_var(char ***argv, int len);

#endif