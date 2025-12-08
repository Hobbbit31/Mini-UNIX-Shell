#include "execute.h"
#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h> // fork, execvp, dup2, _exit, close
#include <sys/types.h>  // pid_t
#include <sys/wait.h>
#include <fcntl.h> // open


int execute_cmd(command_t *cmd){


    if(!cmd->argv || !cmd->argv[0]){
        printf("execute.c: No command to execute\n");
        return 0;
    }

    pid_t pid = fork();

    if(pid == 0){
        // child process

        signal(SIGINT, SIG_DFL);   // child gets normal Ctrl-C

        if(cmd->infile){
            int fd = open(cmd->infile, O_RDONLY);
            if(fd < 0){
                perror("open infile(execute.c)");
                _exit(1);
            }
            dup2(fd, 0); //redirect stdin to infile as zero rep the stdin and fd is the file descriptorof the previosly opened file
            close(fd); //close the file descriptor after duplicating as , if not closed it will leak the file descriptor

        }
        if(cmd->outfile){
            int flags = O_WRONLY | O_CREAT;
            if(cmd->append){
                flags |= O_APPEND;
            } else {
                flags |= O_TRUNC;
            }

            int fd = open(cmd->outfile, flags, 0666);
            if(fd < 0){
                perror("open outfile(execute.c)");
                _exit(1);
            }
            dup2(fd, 1); // redirect stdout to outfile as 1 rep the stdout
            close(fd); //close the file descriptor after duplicating as , if not closed it will leak the file descriptor
        }

        execvp(cmd->argv[0], cmd->argv);
        perror("execvp(inside ex)"); // if execvp returns, there was an error
        _exit(1);
        
    }
    else if(pid > 0){

        if(cmd->background){
            printf("bg pid %d\n", pid);
            
        }else{
            int status;
            waitpid(pid, &status, 0);
        }
        
        return 0;
    }
    else{
        perror("fork");
        return -1;
    }
}


// for executing a pipeline  of two cmd , it basically call the execute cmd for both the cmd after setting up the pipe
int run_pipeline(command_t *left, command_t *right){
    if(left == NULL || right == NULL){
        printf("execute.c: pipeline cmd are null(execute.c)\n");
        return -1;
    }
    int pipefd[2];
    if(pipe(pipefd)<0){
        perror("pipe(execute.c)");
        return -1;
    }


    // left child  creatifn basicallly check if created properly or not
    pid_t left_pid = fork();
    if(left_pid < 0){
        perror("fork for left cmd failed(execute.c)");
        close(pipefd[0]);
        close(pipefd[1]);
        return -1;
    }else if(left_pid == 0){

        signal(SIGINT, SIG_DFL);   // child gets normal Ctrl-C
        if(left->infile != NULL){
            int fd = open(left->infile, O_RDONLY);
            if(fd < 0){
                perror("open infile(execute.c)");
                _exit(1);
            }
            if(dup2(fd, 0) < 0){
                perror("dup2 infile(execute.c)");
                _exit(1);
            }
            close(fd);
        }
    
        if(dup2(pipefd[1], 1) < 0){
            perror("dup2 pipe write end(execute.c)");
            _exit(1);
        }
        close(pipefd[0]);
        close(pipefd[1]);


        if(left->argv != NULL && left->argv[0] != NULL){
             execvp(left->argv[0], left->argv);
        }
        perror("execvp left cmd(execute.c)");
        _exit(1);
    }


    // right child creation , basically check if created properly or not
    pid_t right_pid = fork();
    if(right_pid < 0){
        perror("fork for right cmd failed(execute.c)");
        close(pipefd[0]);
        close(pipefd[1]);

        int tmp_status;
        waitpid(left_pid, &tmp_status, 0);
        return -1;
    }else if(right_pid == 0){

        signal(SIGINT, SIG_DFL);   // child gets normal Ctrl-C

        

        if(dup2(pipefd[0], 0) < 0){
            perror("dup2 pipe read end(execute.c)");
            _exit(1);
        }
        close(pipefd[0]);
        close(pipefd[1]);
        if(right->outfile != NULL){
            int flags = O_WRONLY | O_CREAT;
            if(right->append){
                flags |= O_APPEND;
            } else {
                flags |= O_TRUNC;
            }

            int fd = open(right->outfile, flags, 0666);
            if(fd < 0){
                perror("open outfile(execute.c)");
                _exit(1);
            }
            if(dup2(fd, 1) < 0){
                perror("dup2 outfile(execute.c)");
                _exit(1);
            }
            close(fd);
        }
        if(right->argv != NULL && right->argv[0] != NULL){
                execvp(right->argv[0], right->argv);
        }
        perror("execvp right cmd(execute.c)");
        _exit(1);
             
    }

    // parent process, closing of all pipes and waiting for children
    close(pipefd[0]);
    close(pipefd[1]);


    if(left->background || right->background){
        printf("bg pid %d %d\n", left_pid, right_pid);
    }else{

        int status;
        waitpid(left_pid, &status, 0);
        waitpid(right_pid, &status, 0);
    }
    return 0;
    



    
}