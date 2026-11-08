#ifndef MYSHELL_SHELL_BUILTIN_HISTORY_H
#define MYSHELL_SHELL_BUILTIN_HISTORY_H

void history(char **argv, int len);
void initalize_history();
void save_hist_to_file();

#endif