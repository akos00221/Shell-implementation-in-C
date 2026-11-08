#include "../include/shell_builtin/myshell_declare.h"
#include "../include/myshell_input_parse.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

variable_list vars = {
    .data = NULL,
    .len = 0,
    .size = 4
};
static variable* search_var(const char *name, int len){
    if (len > 0){
        for (int i = 0; i < vars.len; i++){
            if (!strncmp(vars.data[i].name, name, len)){
                return &vars.data[i];
            }
        }
    }
    else{
        for (int i = 0; i < vars.len; i++){
            if (!strcmp(vars.data[i].name, name)){
                return &vars.data[i];
            }
        }
    }

    return NULL;
}
static int validate_var_name(const char *name, int len){
    if (!isalpha(name[0]) && name[0] != '_'){
        return 0;
    }
    for (int i = 0; i < len; i++){
        if (!isalnum(name[i]) && name[i] != '_'){
            return 0;
        }
    }
    return 1;
}
void declare(char **argv, int len){
    if (len > 1){

        if (len > 2){
            variable *ret = search_var(argv[2], -1);
            if (ret){
                printf("declare -- %s=\"%s\"\n", ret->name, ret->val);
            }
            else{
                printf("declare: %s: not found\n", argv[2]);
            }

        }
        char *ind = NULL;

        ind = strchr(argv[1], (int)'=');
        if (ind){

            int name_len = ind-argv[1];
            if (!validate_var_name(argv[1], name_len)){
                printf("declare: `%s': not a valid identifier\n", argv[1]);
                return;
            }
            variable *ret = search_var(argv[1], name_len);

            if (ret) {
                //printf("%s %s\nn", ret->name, ret->val);
                free(ret->val);
                ret->val = strdup(ind+1);
                ret->val_len = strlen(ret->val);
                return;
            }

            if (vars.len == 0 || vars.size == vars.len){
                variable *temp = realloc(vars.data, vars.size*2*sizeof(variable));
                if (!temp) {
                    fprintf(stderr, "Memory allocation for variables failed.\n");
                    return;
                }
                
                vars.size *= 2;
                vars.data = temp;
            }
            
            char *temp = malloc(sizeof(char) * (name_len+1)); 
            if (!temp){
                fprintf(stderr, "Memory allocation for a variable failed.\n");
                return;
            }
            vars.data[vars.len].name = temp;
            strncpy(vars.data[vars.len].name, argv[1], name_len);

            vars.data[vars.len].name[name_len] = '\0';
            vars.data[vars.len].name_len = name_len;

            vars.data[vars.len].val = strdup(ind+1);
            vars.data[vars.len].val_len = strlen(vars.data[vars.len].val);

            //printf("%s %s\n", vars.data[vars.len].name,vars.data[vars.len].val);

            vars.len++;
        }
    }
}

void expand_var(char *argv, char (*token_buffer)[INPUT_BUFFER_SIZE], int *token_buffer_len, int *j){
    if (argv[*j] == '\0') return;
    char c;
    int name_len = 0;
    char name[INPUT_BUFFER_SIZE];
    if (argv[*j] == '{'){
        (*j)++;
        while ((c = argv[*j]) != '\0'){
            if (c == '}'){
                break;
            }
            name[name_len++] = c;
            (*j)++;
        }
    }
    else{
        while ((c = argv[*j]) != '\0'){
            if (isspace(c)){
                break;
            }
            name[name_len++] = c;

            (*j)++;

        }
        (*j)--;

    }
    //printf("%s %d\n", name, name_len);
    variable *ret = search_var(name, name_len);
    if (ret){
        strncpy((*token_buffer)+(*token_buffer_len), ret->val, ret->val_len);
        (*token_buffer_len) += ret->val_len;
    }
    else {
        return;
    }
}
