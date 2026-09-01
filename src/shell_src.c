#include "shell_header.h"
#include <readline/readline.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/wait.h>
#include <ctype.h>

// const char double_quotes_escape_chars[ESCAPE_CHARS_LEN] = {'\\','\"','$','`'};

static const unsigned char char_lookup[256] = {
	[' '] = 1, ['\t'] = 1, ['\n'] = 1, ['\\'] = 2, ['\"'] = 2, ['$'] = 2, ['`'] = 2};

const builtin_command commands[CMD_CNT] = {
	{&(complete), "complete"},
	{&(echo), "echo"},
	{&(exit_shell), "exit"},
	{&(type), "type"}

};
/*
int compare_exec_name(const void *exec1, const void *exec2){
	char *name1 = ((executable*)exec1)->name;
	char *name2 = ((executable*)exec2)->name;
	return strcmp(name1, name2);
}
*/
int exec_size = 3000, exec_len = 0;
executable *executables = NULL;

int completions_size = 16 / 2, completions_len = 0;
executable *custom_completions = NULL;

void parse_path_exec()
{
	executables = malloc(exec_size * sizeof(executable));

	const char *env = getenv("PATH");
	if (env == NULL)
	{
		fprintf(stderr, "No path variables.\n");
		return;
	}
	char *env_cpy = strdup(env);
	if (env_cpy == NULL || executables == NULL)
	{
		fprintf(stderr, "Memory allocation failed.\n");
		return;
	}

	char *token, *saveptr;
	token = strtok_r(env_cpy, ":", &saveptr);

	while (token != NULL)
	{
		DIR *dir = opendir(token);
		if (dir == NULL)
		{
			token = strtok_r(NULL, ":", &saveptr);
			continue;
		}
		int dirid = dirfd(dir);
		struct dirent *entry;
		int token_len = strlen(token);
		while ((entry = readdir(dir)) && entry != NULL)
		{
			if (!faccessat(dirid, entry->d_name, X_OK, 0))
			{
				if (exec_len == exec_size)
				{
					executable *temp = realloc(executables, 2 * exec_size * sizeof(executable));
					if (temp == NULL)
					{
						fprintf(stderr, "Memory allocation failed.\n");
						return;
					}
					exec_size *= 2;
					executables = temp;
				}
				executables[exec_len].path = (char *)malloc(token_len + strlen(entry->d_name) + 2);
				if (executables[exec_len].path == NULL)
				{
					fprintf(stderr, "Memory allocation error.\n");
					return;
				}
				executables[exec_len].name = strdup(entry->d_name);
				sprintf(executables[exec_len].path, "%s/%s", token, executables[exec_len].name);

				exec_len++;
			}
		}
		closedir(dir);
		token = strtok_r(NULL, ":", &saveptr);
	}
	// qsort(executables, exec_len, sizeof(executable), compare_exec_name);
	free(env_cpy);
}

int search_cmd(char *key)
{
	int high = CMD_CNT - 1;
	int low = 0;
	while (low <= high)
	{
		int mid = low + (high - low) / 2;
		if (!strcmp(commands[mid].name, key))
			return mid;
		else if (strcmp(commands[mid].name, key) < 0)
			low = ++mid;
		else
		{
			high = --mid;
		}
	}
	return -1;
}

void exit_shell(char **argv, int len)
{

	for (int i = 0; i < completions_len; ++i)
	{
		free(custom_completions[i].name);
		free(custom_completions[i].path);
	}
	free(custom_completions);

	for (int i = 0; i < exec_len; ++i)
	{
		free(executables[i].name);
		free(executables[i].path);
	}
	free(executables);

	for (int i = 0; i < len; ++i)
		free(argv[i]);
	free(argv);

	exit(EXIT_SUCCESS);
}
void echo(char **argv, int len)
{
	for (int i = 1; i < len && argv[i] != NULL; ++i)
	{
		printf("%s ", argv[i]);
	}
	printf("\n");
}
void complete(char **argv, int len)
{

	switch (argv[1][1])
	{
	case 'p':
		for (int i = 0; i < completions_len; ++i)
		{
			if (!strcmp(custom_completions[i].name, argv[2]))
			{
				printf("complete -C '%s' %s\n", custom_completions[i].path, custom_completions[i].name);
				return;
			}
		}
		printf("complete: %s: no completion specification\n", argv[2]);
		break;
	case 'C':
		// printf("%s %s %s %s\n", argv[0], argv[1], argv[2], argv[3]);
		if (completions_len <= completions_size)
		{
			executable *temp = realloc(custom_completions, completions_size * 2 * sizeof(executable));
			if (temp == NULL)
			{
				fprintf(stderr, "Failed to allocate memory.\n");
				return;
			}
			custom_completions = temp;
			completions_size *= 2;
		}
		custom_completions[completions_len].path = strdup(argv[2]);
		custom_completions[completions_len++].name = strdup(argv[3]);
		break;
	}
}
static char *search_exec(char *argv, int len)
{
	// executable key = {.name = argv, .path = ""};
	for (int i = 0; i < exec_len; ++i)
	{
		if (!strcmp(argv, executables[i].name))
		{
			return strdup(executables[i].path);
		}
	}
	// executable *res = (executable*)bsearch(&key, executables, exec_len, sizeof(executable), compare_exec_name);
	// if (res){
	//	return strdup(res->path);
	// }
	return NULL;
}

void type(char **argv, int len)
{

	int ind = search_cmd(argv[1]);
	if (ind == -1)
	{
		char *full_path = search_exec(argv[1], strlen(argv[1]));
		if (full_path != NULL)
		{
			printf("%s is %s\n", argv[1], full_path);
			free(full_path);
		}
		else
		{
			printf("%s: not found\n", argv[1]);
		}
	}
	else
	{
		printf("%s is a shell builtin\n", argv[1]);
	}
}
int launch_exec(char **argv, int len)
{
	char *full_path = search_exec(argv[0], strlen(argv[0]));

	if (full_path == NULL)
	{
		// printf("%s\n", full_path);
		return -1;
	}

	pid_t id = fork();
	if (id > 0)
	{
		int status;
		waitpid(id, &status, 0);
		free(full_path);
		return 0;
	}
	else if (id == 0)
	{
		execv(full_path, argv);
	}
	else
	{
		free(argv);
		free(full_path);
		printf("bruh");
	}
}
static void create_token(char ***input_tok, int *size, int *len, char token_buffer[], int *token_buffer_len)
{
	token_buffer[*token_buffer_len] = '\0';
	if (*len >= *size)
	{
		char **temp = realloc(*input_tok, (*size) * 2 * sizeof(char *));
		if (temp == NULL)
		{
			fprintf(stderr, "Memory allocation error.\n");
			return;
		}
		(*size) *= 2;
		(*input_tok) = temp;
	}
	(*input_tok)[*len] = strdup(token_buffer);
	*token_buffer_len = 0;
	(*len)++;
}
void parse_input(char *input, char ***input_tok, int *size, int *len)
{
	char c;
	char token_buffer[INPUT_BUFFER_SIZE] = {0};
	parse_states state = NORMAL;
	int j = 0, token_buffer_len = 0;
	*len = 0;
	while ((c = input[j]) != '\0')
	{

		switch (state)
		{
		case NORMAL:

			if (char_lookup[(unsigned char)c] & 1)
			{
				if (token_buffer_len != 0)
				{
					create_token(input_tok, size, len, token_buffer, &token_buffer_len);
				}
			}
			else if (c == '\\')
			{
				state = NORMAL_ESC;
			}
			else if (c == '\'')
			{
				state = SINGLE_QUOTE;
			}
			else if (c == '\"')
			{
				state = DOUBLE_QUOTE;
			}
			else
			{
				token_buffer[token_buffer_len++] = c;
			}

			break;
		case NORMAL_ESC:
			token_buffer[token_buffer_len++] = c;
			state = NORMAL;
			break;
		case SINGLE_QUOTE:
			if (c == '\'')
			{
				if (input[j + 1] == '\'' || input[j - 1] == '\'')
				{
					state = NORMAL;
				}
				else
				{
					create_token(input_tok, size, len, token_buffer, &token_buffer_len);
					state = NORMAL;
				}
			}
			else
			{
				token_buffer[token_buffer_len] = c;
				token_buffer_len++;
			}
			break;
		case DOUBLE_QUOTE:

			if (c == '\"')
			{
				if (input[j + 1] == '\"' || input[j - 1] == '\"' || !isspace(input[j + 1]))
				{
					state = NORMAL;
				}
				else
				{
					create_token(input_tok, size, len, token_buffer, &token_buffer_len);
					state = NORMAL;
				}
			}
			else if (c == '\\')
			{
				state = DOUBLE_QUOTE_ESC;
			}
			else
			{
				token_buffer[token_buffer_len++] = c;
			}

			break;
		case DOUBLE_QUOTE_ESC:
			if (char_lookup[(unsigned char)c] > 0)
			{
				token_buffer[token_buffer_len++] = c;
			}
			state = DOUBLE_QUOTE;
			break;
		}

		j++;
	}
	if (token_buffer_len > 0)
	{
		create_token(input_tok, size, len, token_buffer, &token_buffer_len);
	}
}
char *command_completion_function(const char *text, int state)
{
	static int list_index = 0, len = 0, stage = 0;
	char *name;

	/* If this is a new word to complete, initialize now.  This includes
		saving the length of TEXT for efficiency, and initializing the index
		variable to 0. */
	if (!state)
	{
		list_index = 0;
		len = strlen(text);
	}

	/* Return the next name which partially matches from the command list. */
	if (stage == 0)
	{
		while (list_index < CMD_CNT)
		{
			name = commands[list_index].name;
			list_index++;

			if (strncmp(name, text, len) == 0)
				return (strdup(name));
		}
		stage = 1;
		list_index = 0;
	}
	if (stage == 1)
	{
		while (list_index < exec_len)
		{
			name = executables[list_index].name;
			list_index++;

			if (strncmp(name, text, len) == 0)
				return (strdup(name));
		}
	}

	stage = 0;

	/* If no names matched, then return NULL. */
	return ((char *)NULL);
}
char *custom_completion_function(const char *text, int state)
{
	static int txt_len = 0, output_size = 16, output_len = 0, match_ind = 0;
	static char **output = NULL;
	
	if (!state)
	{
		char *cmd = strndup(rl_line_buffer, strcspn(rl_line_buffer, " \t"));
		match_ind = 0;
		txt_len = strlen(text);
		int cmd_len = strlen(cmd);
		for (int i = 0; i < completions_len; ++i)
		{
			if (!strncmp(cmd, custom_completions[i].name, cmd_len))
			{
				int pipefd[2] = {0};
				
				if (pipe(pipefd) < 0) break;
				pid_t id = fork();
				if (id > 0)
				{
					close(pipefd[1]);
					output = malloc(output_size * sizeof(char *));
					// if null
					for (int i = 0; i < output_size; i++) output[i] = NULL; 
					size_t s = 0; 
					int getline_ind;
					FILE *output_ptr = fdopen(pipefd[0], "r");
					while (( getline_ind = getline(&output[output_len], &s, output_ptr)) && getline_ind != -1)
					{
						output[output_len++][getline_ind-1] = '\0';
						//if len > size realloc
						s = 0;
					}
					int status;
					waitpid(id, &status, 0);
					fclose(output_ptr);
					break;
				}
				else if (id == 0)
				{
					close(pipefd[0]);
					dup2(pipefd[1], STDOUT_FILENO);
					close(pipefd[1]);

					char *args[] = {custom_completions[i].path, NULL};
					execv(custom_completions[i].path, args);
				}
				else
				{
					fprintf(stderr, "Subprocess failed.\n");
					return NULL;
				}
			}
		}
		free(cmd);
	}

	for (; match_ind < output_len; match_ind++){
		//printf("%s\n", output[i]);
		char *current = output[match_ind++];
		if (!strncmp(text, current, txt_len)){
			return strdup(current);
		}
	}
	for (int i = 0; i < output_len; ++i) free(output[i]);
	output_len = 0;
	free(output);
	return ((char *)NULL);
}
void redirect_stdout(char **argv, int len) {}
void initialize_readline(void)
{
	/* Allow conditional parsing of the ~/.inputrc file. */
	rl_readline_name = "myshell";

	/* Tell the completer that we want a crack first. */
	rl_attempted_completion_function = myshell_completion;
}

char **myshell_completion(const char *text, int start, int end)
{
	rl_attempted_completion_over = 1;

	/* If this word is at the start of the line, then it is a command
	   to complete.  Otherwise it is the name of a file in the current
	   directory. */
	if (start == 0)
	{
		return rl_completion_matches(text, command_completion_function);
	}
	char *cmd = strndup(rl_line_buffer, strcspn(rl_line_buffer, " \t"));
    int cmd_len = strlen(cmd);
    int has_custom = 0;

    for (int i = 0; i < completions_len; ++i)
    {
        if (!strncmp(cmd, custom_completions[i].name, cmd_len))
        {
            has_custom = 1;
            break; 
        }
    }
    free(cmd); 

    if (has_custom)
    {
        return rl_completion_matches(text, custom_completion_function);
    }
	return rl_completion_matches(text, rl_filename_completion_function);
}
