#ifndef MYSHELL_SHELL_UTILS_H
#define MYSHELL_SHELL_UTILS_H

#include <unistd.h>

typedef struct {
	char *name, *path;
}executable;

typedef struct {
	size_t exec_size, exec_len;
	executable *executables_table;
}executables;

extern executables execs;

int search_cmd(char *key);
int launch_exec(char **argv, int len);
void parse_path_exec();
void launch(char **argv, int argv_len);
char *search_exec(char *argv, int len);

#endif