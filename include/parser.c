#include "parser.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

//intialiseation of the strucure cmd
void init_cmd(command_t *cmd) {
    cmd->argv = NULL;
    cmd->infile = NULL;
    cmd->outfile = NULL;
    cmd->append = 0;
    cmd->background = 0;
}

//free the memory, so a function whsould be define for that
void free_memory_cmd(command_t *cmd) {
    if (cmd->argv) {
        for (int i = 0; cmd->argv[i] != NULL; i++) {
            free(cmd->argv[i]);
        }
        free(cmd->argv);
    }
    if (cmd->infile) {
        free(cmd->infile);
    }
    if (cmd->outfile) {
        free(cmd->outfile);
    }
    cmd->append = 0;
    cmd->background = 0;
}

int parse_tokens(char *tokens[], int nummber_of_tokens, command_t *cmd){
    init_cmd(cmd);
    int argument_count = 0;
    int i = 0; //fortraversing through 

    // for(int j = 0 ; j<nummber_of_tokens ; j++){
    //     if(strcmp(tokens[j], "&") == 0){
    //         printf("Parser.c error : & should be at the end of the command.\n");
    //         free_memory_cmd(cmd);
    //         return -1;
    //     }
    // }



    // first pass: count arguemtn and assifn the storeage 
    while(i < nummber_of_tokens){

        
        if((strcmp(tokens[i], "<")) == 0 || strcmp(tokens[i], ">") == 0|| strcmp(tokens[i], ">>") == 0){

            if(i + 1 >= nummber_of_tokens){
                fprintf(stderr, "Parser.c error : Missing filename after %s\n", tokens[i]);
                free_memory_cmd(cmd);
                return -1;
            }
            if(strcmp(tokens[i], "<") == 0){
                if(cmd->infile != NULL){
                    fprintf(stderr, "Parser.c error : Multiple input redirections\n");
                    free_memory_cmd(cmd);
                    return -1;
                }
                cmd->infile = strdup(tokens[i + 1]);
            } else {
                if(cmd->outfile != NULL){
                    fprintf(stderr, "Parser.c error : Multiple output redirections\n");
                    free_memory_cmd(cmd);
                    return -1;
                }
                cmd->outfile = strdup(tokens[i + 1]);
                // set append flag only for >>
                if (strcmp(tokens[i], ">>") == 0) {
                    cmd->append = 1;   // append mode
                } else {
                    cmd->append = 0;   // truncate mode
                }

            }
            i += 2; 
        } else {
            argument_count++;
            i++;
        }
    }

    if(argument_count == 0){
        fprintf(stderr, "Parser.c error : No command provided\n");
        free_memory_cmd(cmd);
        return -1;
    }
    cmd->argv = malloc((argument_count + 1) * sizeof(char *)); 

    // second pass : fill the argv array
    i = 0;
    int arg_index = 0;

    while(i < nummber_of_tokens){

        if (strcmp(tokens[i], "<<") == 0) {
            fprintf(stderr, "Parser.c error : here-document '<<' not supported\n");
            free_memory_cmd(cmd);
            return -1;
        }
        if((strcmp(tokens[i], "<")) == 0 || strcmp(tokens[i], ">") == 0 || strcmp(tokens[i], ">>") == 0){
            i += 2; 
        } else {
            cmd->argv[arg_index] = strdup(tokens[i]);
            arg_index++;
            i++;
        }
    }
    cmd->argv[arg_index] = NULL;
    return 0;

}


int parse_line_to_check_pipeline(char *tokens[], int n, command_t *left_cmd, command_t *right_cmd, int *is_pipeline){


    int backg =0 , chBG = -1;

    for(int j =0; j < n; j++){
        if(strcmp(tokens[j], "&") == 0){

            if(chBG != -1){
                fprintf(stderr, "Parser.c error : Multiple & operators found.\n");
                return -1;
            }
            chBG = j;
            
        }
    }

    if(chBG != -1){

        if(chBG != n - 1){
            fprintf(stderr, "Parser.c error : & should be at the end of the command.\n");
            return -1;
        }
        backg = 1;
        n--;
        if(n == 0){
            perror("Parser.c error : no command before & .\n");
            return -1;
        }
    }


        int piperlineindex = -1;

        for(int i = 0; i < n; i++){
            if(strcmp(tokens[i], "|") == 0){
                // check for multiple pipelines
                if(piperlineindex != -1){
                    fprintf(stderr, "(parser.c)Multiple pipelines not supported\n");
                    return -1;
                }
                piperlineindex = i;
                
                
            }
        }

        if(piperlineindex == -1){
            *is_pipeline = 0;
            if(parse_tokens(tokens, n, left_cmd)<0){
                return -1;
            }

            left_cmd->background = backg;
            return 0;
            
        }


        if(piperlineindex == 0 || piperlineindex == n - 1){
            fprintf(stderr, "parser.c error: pipelin | can,t be at the beg or the end.\n");
            return -1;
        }

        *is_pipeline = 1;
        int left_size = piperlineindex;
        int right_size = n - piperlineindex - 1;


        if(parse_tokens(tokens, left_size, left_cmd)<0){
            return -1;
        }

        if(parse_tokens(tokens + left_size + 1, right_size, right_cmd)<0){

            free_memory_cmd(left_cmd);
            return -1;

        }


        // bith the process will run in backgrond if & is present at end    
        // cmd & | cmd will give error in the first place due to & position check
        left_cmd->background = backg;
        right_cmd->background = backg;
        return 1;

}

