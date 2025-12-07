#include "execute.h"
#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // fork, execvp, dup2, _exit, close
#include <sys/types.h>  // pid_t
#include <sys/wait.h>
#include <fcntl.h> // open


int execute_cmd(command_t *cmd){


    if(!cmd->argv || !cmd->argv[0]){
        fprintf(stderr, "Error: No command to execute\n");
        return 0;
    }

    pid_t pid = fork();

    if(pid == 0){
        // child process

        if(cmd->infile){
            int fd = open(cmd->infile, O_RDONLY);
            if(fd < 0){
                perror("open infile");
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
                perror("open outfile");
                _exit(1);
            }
            dup2(fd, 1); // redirect stdout to outfile as 1 rep the stdout
            close(fd); //close the file descriptor after duplicating as , if not closed it will leak the file descriptor
        }

        execvp(cmd->argv[0], cmd->argv);
        perror("execvp"); // if execvp returns, there was an error
        _exit(1);
        
    }
    else if(pid > 0){
        int status;
        waitpid(pid, &status, 0);
        return 0;
    }
    else{
        perror("fork");
        return -1;
    }
}