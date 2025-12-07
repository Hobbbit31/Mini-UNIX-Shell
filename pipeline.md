You want to run:
cmd1 | cmd2

Meaning:
cmd1 runs, writes its output to a pipe (instead of the terminal)
cmd2 runs, reads its input from that pipe (instead of the keyboard)
run_pipeline(left, right, background) will:
create the pipe
fork two children
set up redirections using dup2
close unused file descriptors
wait for children if not running in background

We’ll go over all required concepts:
pipe()
fork()
file descriptors & inheritance
dup2() (redirection)
close()
execvp() (to actually run cmd)
waitpid()

background vs foreground logic



1. pipe() – Creating a unidirectional data channel
Signature:
int pipe(int fd[2]);
On success: fills fd[0] and fd[1], returns 0
On error: returns -1
Meaning of fd:
fd[0] → read end of the pipe
fd[1] → write end of the pipe
Data you write(fd[1], ...) can be read(fd[0], ...).
Important properties:
Pipes are unidirectional: one side writes, other reads.
They are inherited across fork(), so children get them too.
A pipe internally behaves like a small buffer in the kernel.
For cmd1 | cmd2:
cmd1 must write into pipe → uses fd[1]
cmd2 must read from pipe → uses fd[0]
So:
int p[2];
if (pipe(p) < 0) {
    perror("pipe");
    // handle error
}

2. fork() – Creating child processes
Signature:
pid_t fork(void);
Returns 0 in the child
Returns > 0 (child PID) in the parent
Returns -1 on error
Key behavior:
Entire process is duplicated – memory, file descriptors, etc.
Both parent and child continue execution from the line after fork().
Why needed for pipeline?
We need two processes running in parallel:
One running cmd1
One running cmd2
So we’ll call fork() twice: once for left (cmd1), once for right (cmd2).


3. File descriptors & inheritance
When we do pipe(p) and fork():
The pipe FDs (p[0], p[1]) are open in:
Parent
Left child
Right child
We must carefully close unused ends in each process to:
avoid resource leaks
avoid “hanging” reads (reader thinks writer still alive)
make EOF work properly
Rule of thumb:
A process that only reads the pipe must close the write end
A process that only writes the pipe must close the read end
Parent usually closes both ends after forking children

4. dup2() – Redirecting stdin/stdout
Signature:
int dup2(int oldfd, int newfd);
Meaning:
Make newfd refer to the same open file description as oldfd.
Steps it does:
If newfd is open, it closes it
Makes newfd a duplicate of oldfd
After that, both oldfd and newfd refer to the same underlying resource
We use well-known FDs:
STDIN_FILENO (0)
STDOUT_FILENO (1)
For pipeline:
In left child (cmd1):
We want its stdout to go to the pipe’s write end:
dup2(p[1], STDOUT_FILENO);  // stdout → pipe write
In right child (cmd2):
We want its stdin to come from the pipe’s read end:
dup2(p[0], STDIN_FILENO);   // stdin → pipe read
After dup2:
printf, puts, write(1, ...) in cmd1 all go into the pipe
scanf, fgets(stdin, ...), read(0, ...) in cmd2 all read from the pipe
Very important: after calling dup2, you should close the original p[0] or p[1] in that child, because you no longer need them (stdout/stdin now point 
to the pipe).

5. close() – Removing FDs
Signature:
int close(int fd);
Releases the FD from the process.
When all processes have closed a given end of the pipe:
if all write ends are closed → readers get EOF
if all read ends are closed → writing may get SIGPIPE or error
In pipeline setup:
In parent:
After forking both children → parent doesn’t need to read/write pipe:
close(p[0]);
close(p[1]);
In left child:
After dup2(p[1], STDOUT_FILENO):
close(p[0]); // never reads
close(p[1]); // original FD, stdout now points to pipe
In right child:
After dup2(p[0], STDIN_FILENO):
close(p[1]); // never writes
close(p[0]); // original FD, stdin now points to pipe
This is crucial so that:
cmd2 can detect EOF when cmd1 finishes and closes its write end.
No FD leaks.

6. execvp() – Actually running the command
Your run_pipeline will not just fork; it must run commands in the children.
Common function:
int execvp(const char *file, char *const argv[]);
Replaces the current process image with a new program.
On success: never returns
On failure: returns -1
Typical code in child:
execvp(cmd->argv[0], cmd->argv);
perror("execvp");
_exit(1);
In pipeline:
Left child:
Set up redirection (dup2 for stdout)
Then execvp(left->argv[0], left->argv)
Right child:
Set up redirection (dup2 for stdin)
Then execvp(right->argv[0], right->argv)


7. waitpid() – Waiting for children (foreground mode)
Signature:
pid_t waitpid(pid_t pid, int *status, int options);
pid can be:
specific PID
-1 → any child
status stores exit info (optional)
options can be 0, or flags like WNOHANG, WUNTRACED
For foreground pipeline (background == 0):
Parent should wait for both children to finish.
You can:
int status;
waitpid(left_pid, &status, 0);
waitpid(right_pid, &status, 0);
Or loop:
while (waitpid(-1, &status, 0) > 0)
    ; // until all children are done
For background pipeline (background != 0):
You do not wait in the usual way.
You might:
Just return immediately
Or store PIDs in a job list and handle them later (if your assignment has jobs)


8. Background vs foreground behavior (background flag)
Your run_pipeline signature:
run_pipeline(command_t *left, command_t *right, int background)
If background == 0 → foreground:
Run pipeline
Parent waits for children with waitpid
If background != 0 → background:
Run pipeline
Parent does NOT block:
It just returns to the main prompt quickly
Children still run in background, writing output etc.
Your minimal behavior:
Foreground: wait for children
Background: don’t wait (maybe print their PIDs)



