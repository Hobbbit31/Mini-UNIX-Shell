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



// directory traversing function later i wiil also implement auto complete feature for this
int directory_traversing(char **argument_Name){
    if(argument_Name[0] == NULL) return 0;
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
    if(argument_Name[0] == NULL) return -1;
    if(strcmp(argument_Name[0], "pwd") != 0){
        return -1;
    }
    char print[100];
    if(getcwd(print, sizeof(print)) != NULL) {
        printf("%s\n", print);
    } else {
        perror("getcwd");
    }
    return 1;
}

// SIGCHLD handler to reap background children
static void sigchld_handler(int sig) {
    (void)sig;
    int saved_errno = errno;
    // Reap all dead children, non-blocking
    while (1) {
        int status;
        pid_t pid = waitpid(-1, &status, WNOHANG);
        if (pid <= 0) break;
        // Optionally print status of background job
        // printf("[bg %d] finished\n", pid);
    }
    errno = saved_errno;
}

// Execute command with support for simple redirection and background (&)
int execute_command(char *argument_list[], int arg_Count) {
    if (arg_Count == 0 || argument_list[0] == NULL) return -1;

    // Parse for redirection and background token(s)
    char *infile = NULL;
    char *outfile = NULL;
    int background = 0;

    // We'll compact the argument list in-place as we remove tokens
    int write_idx = 0;
    for (int read_idx = 0; read_idx < arg_Count; ++read_idx) {
        char *tok = argument_list[read_idx];
        if (strcmp(tok, "<") == 0) {
            // input redirection
            if (read_idx + 1 < arg_Count) {
                infile = argument_list[read_idx + 1];
                read_idx++; // skip filename
            } else {
                fprintf(stderr, "Syntax error: no input file after '<'\n");
                return -1;
            }
        } else if (strcmp(tok, ">") == 0) {
            // output redirection (truncate)
            if (read_idx + 1 < arg_Count) {
                outfile = argument_list[read_idx + 1];
                read_idx++; // skip filename
            } else {
                fprintf(stderr, "Syntax error: no output file after '>'\n");
                return -1;
            }
        } else if (strcmp(tok, "&") == 0) {
            // background token; we'll allow & anywhere but treat it as flag
            background = 1;
            // do not copy '&' into new argv
        } else {
            // regular argument; keep it
            argument_list[write_idx++] = argument_list[read_idx];
        }
    }
    // Null-terminate compacted argv
    argument_list[write_idx] = NULL;
    int new_argc = write_idx;

    if (new_argc == 0) {
        // Nothing to run (e.g., line was just "cmd > out" with no cmd)
        fprintf(stderr, "Error: no command to execute\n");
        return -1;
    }

    // Fork and execute
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return -1;
    } else if (pid == 0) {
        // Child
        // If infile specified, open and dup2 to STDIN
        if (infile) {
            int fd = open(infile, O_RDONLY);
            if (fd < 0) {
                perror("open input file");
                exit(1);
            }
            if (dup2(fd, STDIN_FILENO) < 0) {
                perror("dup2 input");
                close(fd);
                exit(1);
            }
            close(fd);
        }
        // If outfile specified, open (create/truncate) and dup2 to STDOUT
        if (outfile) {
            int fd = open(outfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd < 0) {
                perror("open output file");
                exit(1);
            }
            if (dup2(fd, STDOUT_FILENO) < 0) {
                perror("dup2 output");
                close(fd);
                exit(1);
            }
            close(fd);
        }

        // Execute
        execvp(argument_list[0], argument_list);
        // If execvp returns, there was an error
        perror("Execution failed");
        exit(1);
    } else {
        // Parent
        if (background) {
            // Do not wait; child runs in background
            printf("[bg] pid %d\n", pid);
            // immediate return; SIGCHLD handler will reap when done
            return 0;
        } else {
            // Foreground: wait for child
            int status;
            if (waitpid(pid, &status, 0) < 0) {
                perror("waitpid");
                return -1;
            }
            return 0;
        }
    }
}

int main() {
    // Install SIGCHLD handler to reap background processes
    struct sigaction sa;
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP; // restart syscalls, don't notify on stopped children
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        // not fatal; continue
    }
    
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

        // Handle builtins first (simple approach: do not support redirection/background for these)
        if (directory_traversing(argument_list)) {
            free_tokens(argument_list, arg_Count);
            free(read);
            continue;
        }
        if (pwd_print(argument_list) == 1) {
            free_tokens(argument_list, arg_Count);
            free(read);
            continue;
        }

        // Execute (handles redirection and background)
        execute_command(argument_list, arg_Count);
        
        // Free allocated token strings
        free_tokens(argument_list, arg_Count);
        free(read);
    }
    
    return 0;
}