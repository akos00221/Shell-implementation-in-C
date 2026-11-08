#include <readline/tilde.h>
#include <stdio.h>
#include <stdlib.h>
#include "../include/shell_builtin/myshell_history.h"
#include <readline/history.h>
#include <string.h>
#include <errno.h>

int num_of_read_hist;

static void print_history(char *arg){
    HIST_ENTRY** hist_list = history_list();
    if (hist_list == NULL){
        return;
    }
    int n = -1, j = 0;
    if (arg) n = atoi(arg);

    if (n == -1){
        while (hist_list[j] != NULL){
            printf("\t%d  %s\n", j+1, hist_list[j]->line);
            j++;
        }
    }
    else{
        for (int i = history_length-n; i < history_length; i++){
            printf("\t%d  %s\n", i+1, hist_list[i]->line);
        }
    }
}

void history(char **argv, int len){
    if (len > 2){
        int ret;
        if (!strncmp(argv[1], "-r", 3)){
            ret = read_history(argv[2]);
            if (ret){
                fprintf(stderr, "Reading history from file failed\n");
            }
        }
        else if (!strncmp(argv[1], "-w", 3)){
            ret = write_history(argv[2]);
            if (ret){
                fprintf(stderr, "Writing history to a file failed\n");
            }
        }
        else if (!strncmp(argv[1], "-a", 3)){
            //static int number_of_written_entries = history_length;
            ret = append_history(history_length-history_lines_written_to_file, argv[2]);
            if (ret){
                fprintf(stderr, "Appending history to a file failed\n");
            }
        }

    }
    else if (len == 2){
        print_history(argv[1]);
    }
    else{
        print_history(NULL);
    }


}
void initalize_history(){
    char *fpath = getenv("HISTFILE");
    if (!fpath || fpath[0] == '\0'){
        return;
    }
    char *expanded_fpath = tilde_expand(fpath);
    int ret = read_history(expanded_fpath);
    num_of_read_hist = history_length;
    if (ret && ret != ENOENT && ret != EINVAL){
        fprintf(stderr, "Reading history on startup from file failed: %s\n", strerror(ret));
    }
    free(expanded_fpath);
}
void save_hist_to_file(){
    char *fpath = getenv("HISTFILE");
    if (!fpath || fpath[0] == '\0'){
        return;
    }    
    char *expanded_fpath = tilde_expand(fpath);
    int ret = append_history(history_length-num_of_read_hist, expanded_fpath);
    if (ret && ret != ENOENT && ret != EINVAL){
        fprintf(stderr, "Appending history to a file failed\n");
    }
    free(expanded_fpath);
}