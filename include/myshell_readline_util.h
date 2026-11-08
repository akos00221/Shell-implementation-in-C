#ifndef MYSHELL_READLINE_UTIL_H
#define MYSHELL_READLINE_UTIL_H

char *custom_completion_function(const char *text, int state);
void initialize_readline(void);
char **myshell_completion(const char *text, int start, int end);

#endif