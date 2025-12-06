#include <stdio.h>    
#include <unistd.h>   
#include <limits.h>   
#include <ctype.h>    
#include <string.h> 
#include <sys/types.h> 
#include <stdlib.h>
#include <sys/wait.h>


int tokenize(const char *line, char *tokens[]) {
    int i = 0;      // position in line
    int t = 0;      // number of tokens

    while (line[i] != '\0') {

        // Skip spaces
        while (line[i] != '\0' && isspace((unsigned char)line[i])) {
            i++;
        }
        if (line[i] == '\0') break;

        // ---------------------------
        // 1. Quoted strings
        // ---------------------------
        if (line[i] == '"' || line[i] == '\'') {
            char quote = line[i];
            int start, len;
            char *word;

            i++;            // skip opening quote
            start = i;

            while (line[i] != '\0' && line[i] != quote) {
                i++;
            }

            len = i - start;
            word = (char *)malloc(len + 1);
            if (!word) return t; // out of memory, return what we have

            memcpy(word, &line[start], len);
            word[len] = '\0';

            tokens[t++] = word;

            if (line[i] == quote) i++;  // skip closing quote
            continue;
        }

        // ---------------------------
        // 2. Operators (&, |, <, >)
        //    Support: &&, ||, >>, <<, and single ones
        // ---------------------------
        if (line[i] == '&' || line[i] == '|' || line[i] == '<' || line[i] == '>') {

            char op[3] = {0, 0, 0}; // max 2 chars + '\0'
            op[0] = line[i];

            // Check two-character operators
            if (line[i + 1] != '\0') {
                if ((line[i] == '&' && line[i+1] == '&') || (line[i] == '|' && line[i+1] == '|') || (line[i] == '<' && line[i+1] == '<') || (line[i] == '>' && line[i+1] == '>')) {
                    op[1] = line[i+1];
                    i += 2;
                } else {
                    i += 1;
                }
            } else {
                i += 1;
            }

            tokens[t++] = strdup(op);
            // if (t >= max) break;
            continue;
        }

        // ---------------------------
        // 3. Normal word
        // ---------------------------
        {
            int start = i;
            int len;
            char *word;

            while (line[i] != '\0' &&  !isspace((unsigned char)line[i]) && line[i] != '&' && line[i] != '|' && line[i] != '<' && line[i] != '>') {
                i++;
            }

            len = i - start;
            if (len <= 0) continue;

            word = (char *)malloc(len + 1);
            if (!word) return t;

            memcpy(word, &line[start], len);
            word[len] = '\0';

            tokens[t++] = word;
        }
    }

    tokens[t] = NULL; // Null-terminate the tokens array
    return t;
}


void free_tokens(char *tokens[], int count) {
    int i;
    for (i = 0; i < count; i++) {
        free(tokens[i]);
    }
}

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

// creates tokes
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

int execution(char **argument_Name, int arg_Count){
    int pid = fork();
    if(pid < 0){
        perror("Fork failed");
        return -1;
    } else if(pid == 0){
        // Child process
        if(execvp(argument_Name[0], argument_Name) < 0){
            perror("Execution failed");
           
            exit(1);
            
        }
    } else {
        // Parent process
        int status;
        waitpid(pid, &status, 0);
    }
    return 0;
}

// directory traversing function later i wiil also implement auto complete feature for this
int directory_traversing(char **argument_Name){
    if(strcmp(argument_Name[0], "cd") == 0){
            char *target_Dir = argument_Name[1]? argument_Name[1] : getenv("HOME");
            if(!target_Dir){
                target_Dir=".";
            }
            if(chdir(target_Dir) != 0){
                perror("chdir failed");
            }
            return 1;        
    } 
    return 0;
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

int main() {
    
    while(1){
        char *read = NULL;
        size_t size = 0;
       char curr_working_dir[4000];
       if(getcwd(curr_working_dir, sizeof(curr_working_dir))){
           printf("%s $ ", curr_working_dir);
       } else 
       printf("$ ");
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
        int arg_Count = tokenize(trimmed_input, argument_list);

        if(arg_Count == 0) {
            free(read);
            continue;
        }
        for(int i = 0; i < arg_Count; i++){
            printf("Argument %d: %s\n", i, argument_list[i]);
        }
        // if(strcmp(argument_Name[0], "cd") == 0){
        //     char *target_Dir = argument_Name[1]? argument_Name[1] : getenv("HOME");
        //     if(!target_Dir){
        //         target_Dir=".";
        //     }
        //     if(chdir(target_Dir) != 0){
        //         perror("chdir failed");
        //     }
        //     continue;
        // } 
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
        int result = execution(argument_list, arg_Count);  
        free_tokens(argument_list, arg_Count);
        free(read);
    }
    
    return 0;
}