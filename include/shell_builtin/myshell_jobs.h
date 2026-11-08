#ifndef  MYSHELL_SHELL_BUILTIN_JOBS_H
#define MYSHELL_SHELL_BUILTIN_JOBS_H

#include <stddef.h>

typedef enum {
	PRINT_ALL,
	PRINT_DONE
}print_style;

typedef enum {
	RUNNING,
	STOPPED,
	DONE,
	NUM_OF_JOB_STATES
}job_states;

typedef struct {
	int job_num, id;
	job_states state;
	int cmd_len;
	char **cmd;
}job;

typedef struct {
	size_t jobs_list_size, jobs_list_len;
	int next_job_num;
	job *data;
}jobs_list;

extern jobs_list jl;
extern const char *job_states_lookup[NUM_OF_JOB_STATES];

void jobs(char **argv, int len);
void mark_jobs();
void print_jobs(print_style ps);
void sweep_jobs();

#endif