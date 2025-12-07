#include <stdio.h>    
#include <unistd.h>   
#include <limits.h>   
#include <ctype.h>    
#include <string.h> 
#include <sys/types.h> 
#include <stdlib.h>
#include <sys/wait.h>
#include "include/parser.h"
#include "include/execute.h"


int tokenize(const char *line, char *tokens[], int max_tokens);
void free_tokens(char *tokens[], int count);

char *trim(char *str) {
    if(str == NULL) {
        return str;
    }
    char *end;
    while(isspace((unsigned char)*str)){
        str++;
    }
    if(*str == 0)  
        return str;
    end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end)) end--;
    *(end + 1) = '\0';
    return str;
}

// int tokens(char *line , char **argument_Name , int maximum_arguements){
//     int arg_Count = 0;
//     char *p = line;
//     while(*p && arg_Count < maximum_arguements - 1){
//         //this below line skips the starting spaces on any token
//         while(isspace((unsigned char)*p)) p++; 
//         // this checks if we have reached the end of the string
//         if(*p == '\0') break; 
//         // token is entered into argument_Name
//         argument_Name[arg_Count++] = p;
//         // move p to the end of the current token
//         while(*p && !isspace((unsigned char)*p)) p++;
//         if(*p){
//             // If whitespace found, turn it into '\0'
//             *p = '\0'; 
//             p++;
//         }
//     }
//     argument_Name[arg_Count] = NULL; 
//     return arg_Count;
// }

// int execution(char **argument_Name, int arg_Count){
//     int pid = fork();
//     if(pid < 0){
//         perror("Fork failed");
//         return -1;
//     } else if(pid == 0){
//         // Child process
//         if(execvp(argument_Name[0], argument_Name) < 0){
//             perror("Execution failed");      
//             _exit(1);            
//         }
//     } else {
//         // Parent process
//         int status;
//         waitpid(pid, &status, 0);
//     }
//     return 0;
// }

// directory traversing function later i wiil also implement auto complete feature for this
int directory_traversing(char **argument_Name){
    if (strcmp(argument_Name[0], "cd") != 0) {
        return 0; // not a cd command
    }

    char *target_Dir = NULL;

    if (argument_Name[1] == NULL || strcmp(argument_Name[1], "~") == 0) {
        target_Dir = getenv("HOME");
    } else {
        target_Dir = argument_Name[1];
    }

    if (target_Dir == NULL)
        target_Dir = ".";

    if (chdir(target_Dir) != 0) {
        perror("cd");
    }

    return 1; // we handled a cd
}

int pwd_print(char **argument_Name){
    if(strcmp(argument_Name[0], "pwd") != 0){
        return -1;
    }
    char print[100];
    getcwd(print, sizeof(print));
    printf("%s\n", print);
    return 1;
}

void getCurrDir(char *buffer, size_t size){
    if(getcwd(buffer, size)){
        printf("%s $ ", buffer);
    } else {
        printf("$ ");
        
    }
}

int main() {
    
    while(1){
        char *read = NULL;
        size_t size = 0;
       char curr_working_dir[4000];

       getCurrDir(curr_working_dir, sizeof(curr_working_dir));
       
        fflush(stdout);

    //    if(fgets(read, sizeof(read), stdin) == NULL) {
    //        printf("\n");
           
    //           break; 
    //         //   break or the work of ctrl + c done by ctrl + d
    //    }
      
       if(getline(&read, &size, stdin) == -1) {
           printf("\n");
           free(read);
           break;
       }
       char *trimmed_input = trim(read);

       if(strlen(trimmed_input) == 0){
            free(read);
           continue;
       }
         if(strcmp(trimmed_input, "exit") == 0) {
              free(read);
              break;
         }

        char *argument_list[100];
        int arg_Count = tokenize(trimmed_input, argument_list, 100);

        int check = directory_traversing(argument_list);
        if(check == 1){
            free(read);
            continue;
        }
        int check_pwd = pwd_print(argument_list);
        if(check_pwd == 1){
            free(read);
            continue;
        }
        if(arg_Count == 0) {
            free(read);
            continue;
        }

        //tokeniser print for debugging
        printf("---- Tokens ----\n");
        for(int i = 0; i < arg_Count; i++){
            printf("Argument %d: %s\n", i, argument_list[i]);
        }
        printf("----------------\n");

        // command_t cmd;
        // if(parse_tokens(argument_list, arg_Count, &cmd) < 0){
        //     free_tokens(argument_list, arg_Count);
        //     free(read);
        //     continue;
        // }
        // // DEBUG: see what parser understood
        // printf("---- Parsed Command ----\n");
        // for (int i = 0; cmd.argv[i] != NULL; i++) {
        //     printf("argv[%d] = %s\n", i, cmd.argv[i]);
        // }
        // printf("infile:  %s\n", cmd.infile  ? cmd.infile  : "(none)");
        // printf("outfile: %s\n", cmd.outfile ? cmd.outfile : "(none)");
        // printf("------------------------\n");

        command_t left , right;
        int is_pipeline = 0;

        int check_pipeline = parse_line_to_check_pipeline(argument_list, arg_Count, &left, &right, &is_pipeline);
        if(check_pipeline < 0){
            free_tokens(argument_list, arg_Count);
            free(read);
            continue;
        }
        
        if(!is_pipeline){
            execute_cmd(&left);
            free_memory_cmd(&left);
            
            
            
        }else{
            // DEBUG: see what parser understood for left command
            printf("---- Left Command ----\n");
            for (int i = 0; left.argv[i] != NULL; i++) {
                printf("argv[%d] = %s\n", i, left.argv[i]);
            }
            printf("infile:  %s\n", left.infile  ? left.infile  : "(none)");
            printf("outfile: %s\n", left.outfile ? left.outfile : "(none)");
            printf("------------------------\n");

            // DEBUG: see what parser understood for right command
            printf("---- Right Command ----\n");
            for (int i = 0; right.argv[i] != NULL; i++) {
                printf("argv[%d] = %s\n", i, right.argv[i]);
            }
            printf("infile:  %s\n", right.infile  ? right.infile  : "(none)");
            printf("outfile: %s\n", right.outfile ? right.outfile : "(none)");
            printf("------------------------\n");

            // Execute pipeline
            // (Implementation of pipeline execution is not shown here)

            run_pipeline(&left, &right);
            
            free_memory_cmd(&left);
            free_memory_cmd(&right);
          
            
            

        }





      
        
        
       
        free_tokens(argument_list, arg_Count);
        free(read);
    }
    
    return 0;
}