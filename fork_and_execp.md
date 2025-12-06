Awesome, let’s go deep. I’ll walk through:
What really happens when you run a command in your shell
What fork() and execvp() do at OS level
How control “comes back” to the shell
How redirection and pipes actually work with file descriptors
What “redirection to parent” really means (and what it doesn’t mean)

1. Big picture: two processes, not one
When you type:
ls -l
into your shell, there are two processes involved:
The shell process (your program)
The child process (running ls)
The key idea:
fork() duplicates your process;
execvp() in the child replaces the child’s code with the new program;
the parent (shell) just waits, then prints another prompt.
Nothing “returns from execvp” in the child on success — instead, the child program itself runs and later exits


2. Timeline: what happens when you run ls
Let’s trace step-by-step inside your shell.
You have code like:

pid_t pid = fork();

if (pid == 0) {
    // child
    execvp(argv[0], argv);
    // only runs if execvp fails
} else {
    // parent (shell)
    int status;
    waitpid(pid, &status, 0);
}
Step 1: fork()
The kernel:
Creates a new process entry in its process table.
Copies the address space (virtual memory, stack, heap etc.) of the parent into the child (modern OS often use copy-on-write).
Copies the file descriptor table (stdin, stdout, stderr, other fds) — they now point to the same underlying files/terminal.
Child gets its own PID; parent keeps its own PID.
After fork() returns:
In the parent: pid > 0 (child’s PID)
In the child: pid == 0
Now you have two processes executing the same code, starting just after fork().
Step 2: execvp(argv[0], argv) in the child
execvp() is a library wrapper around the kernel syscall execve. Internally, it roughly does:
1. Look up argv[0] in the PATH environment variable (e.g. /bin, /usr/bin, …).
2. Build the full path like /bin/ls.
3. Call execve("/bin/ls", argv, environ).
Now the important part:
execve does not create a new process. It replaces the current process’s program.
Deeply, the kernel does:
1. Throw away the existing program image of the child:
    Old code segments
    Old static data
    Old stack, heap (logically; pages are replaced)
2. Load the new program (ls binary):
    Read the ELF header (or other executable format).
    Map text (code) and data sections into the child’s address space.
    Set up a new stack (with argc, argv, envp).
3. Keep the same PID (child’s PID doesn’t change).
4. Keep the same open file descriptors (stdin, stdout, stderr, etc.)
    → This is crucial for redirection and pipelines.
5. Set the instruction pointer to the entry point of the program (like _start), which eventually calls main().
After execve succeeds:
    The old child code is gone.
    Now it’s as if the process was always an ls process.
    execvp() never returns in the child (on success). Control just jumps into the new program.
If execvp fails (e.g., command not found):
    execve returns -1, sets errno.
    Then your code after execvp() runs and you print an error.


3. So how do we “come back” to the shell?
Key point:
The shell never goes away. Only the child is replaced.
Look again at the structure:
if (pid == 0) {
    // Child: execvp(...)
} else {
    // Parent (shell): waitpid(...)
}
1. The parent continues running your shell code.
2. It calls waitpid(pid, &status, 0) to block until the child finishes.
Detail: what does waitpid do?
1. The parent process goes into a blocked state, waiting for child’s exit.
2. The kernel keeps track of:
    The child’s PID
    Exit status (or termination signal)
3. When the child:
    Returns from main() or calls exit() → process ends.
    Or is killed by a signal like SIGINT → process ends.
4. The kernel:
    Marks child as a zombie temporarily (its entry stays so parent can read exit code).
    Wakes up the parent blocked in waitpid.
    Gives the parent the child’s status.
    Once the parent calls waitpid, the kernel can remove the child entry (reap the zombie).
5. After waitpid returns:
    Your shell code resumes.
    You print the next prompt.
    From user perspective: “we came back from the command”.
So we “return to shell” not from execvp, but because:
The child process exit wakes up the parent (shell) which was waiting in waitpid.


4. How standard I/O (stdin, stdout, stderr) works
Each process has a file descriptor table — a small array that maps integer fd → open file object.
By convention:
0 → STDIN_FILENO (standard input)
1 → STDOUT_FILENO (standard output)
2 → STDERR_FILENO (standard error)
In a typical terminal:
1. Your shell starts with:
    fd 0,1,2 all pointing to the terminal device (e.g. /dev/pts/3):
        Input: keystrokes from you
        Output: text on the screen
When you call fork():
    The child’s fd table is a copy of the parent’s.
    So child also has:
        fd 0 → terminal
        fd 1 → terminal
        fd 2 → terminal
So if ls writes to stdout (fd 1), the kernel writes those bytes to the same terminal device, and you see them on screen.


5. How redirection works with dup2 (super important concept)
Redirection is about changing which file descriptor 1 (stdout) or 0 (stdin) points to before execvp.
Imagine you run:
    ls > out.txt
What your shell does:
1. Parse ls > out.txt:
    Command: ls
    Arg list: ["ls", NULL]
    Output redirection: filename = out.txt
2. In the child (after fork() and before execvp):

int fd = open("out.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
if (fd < 0) { perror("open"); _exit(1); }
// Redirect stdout to this file
dup2(fd, STDOUT_FILENO);  // Now fd 1 refers to out.txt
close(fd);                // we can close the original fd now

What does dup2(fd, STDOUT_FILENO) actually do?
    It makes descriptor number 1 (stdout) refer to the same underlying open file as fd.
    If there was already something on fd 1 (the terminal), it closes that link and replaces it.
    After this:
        When the child calls write(1, ...) (via printf, etc.), the bytes go to out.txt instead of the terminal.
Then the child does:
    execvp("ls", argv);
    ls starts running with stdout already pointing to out.txt.
    ls doesn’t know or care about redirection; it just writes to stdout.
The parent’s descriptors are unchanged → parent’s stdout is still the terminal.


6. How pipes work: data flow between processes, not to parent
Now consider:
    ls | grep foo
Here’s what your shell does:
1. Creates a pipe:
    int pipefd[2];
    pipe(pipefd);   // pipefd[0] = read end, pipefd[1] = write end

2. Forks left child (for ls):
pid_t p1 = fork();
if (p1 == 0) {
    // child 1 (ls)
    dup2(pipefd[1], STDOUT_FILENO); // stdout -> pipe write end
    close(pipefd[0]);
    close(pipefd[1]);
    execvp("ls", left_argv);
    _exit(127);
}

3. Forks right child (for grep):
pid_t p2 = fork();
if (p2 == 0) {
    // child 2 (grep)
    dup2(pipefd[0], STDIN_FILENO);  // stdin <- pipe read end
    close(pipefd[1]);
    close(pipefd[0]);
    execvp("grep", right_argv);
    _exit(127);
}


4. Parent closes both ends of the pipe:
close(pipefd[0]);
close(pipefd[1]);


5. Parent waits for both children (unless background).
Data flow:
    ls writes to its stdout → which is now the write end of the pipe.
    Kernel buffers that data.
    grep reads from its stdin → which is now the read end of the pipe.
    Nothing is “redirected to parent” — data flows directly from ls → kernel pipe buffer → grep.

The parent rarely reads the output itself (unless you’re writing something like $(...) command substitution). Standard shell pipelines just connect child processes.

7. What does “redirection to parent” actually mean?
In normal shell pipelines output is NOT sent to the parent. It goes:
    To the terminal (no redirection)
    To a file (> redirection)
    To another process (pipes)
The parent (your shell) mostly:
    Sets up fds.
    Forks.
    Calls execvp in child.
    Calls waitpid.
    Does not handle the actual data of stdout/stderr.
There are special cases like capturing output in a variable (in advanced shells), but that’s not your assignment’s core.
So when people casually say “output comes back to the shell”, they usually mean:
When the child stops, the user sees the prompt of the shell again.
Not that stdout content literally flows into the shell process (unless you design it that way).


8. Putting it all together: mental model
Here’s a good mental picture:
Running a simple command
    Shell: reads input, parses → ["ls", "-l"].
    Shell: fork().
    Child:
        Maybe set up redirection/pipes (dup2, close).
        execvp("ls", argv) → kernel loads ls.
        ls runs, writes to stdout.
Parent (shell):
    waitpid(child).
    When child exits, waitpid returns.
    Shell prints new prompt.
Why execvp doesn't disturb the parent
    execvp only affects the child process.
    The parent continues running the same shell program, blocked in waitpid or doing other stuff.
    So after the child ends, control is already in the parent shell, ready to read the next line.





My question: so in child process when the new binary is placed, how it return to parent
ans : The child NEVER “returns” to the parent after exec. The parent only wakes up after the child EXITS.

🧠 1. First, understand this: A child does NOT return from execvp()
When you do:

pid_t pid = fork();
if (pid == 0) {
    execvp(...);   // child
}

After execvp() succeeds:
    The child’s entire memory image is replaced by the new binary (ls, grep, etc.)
    The child never runs your C code again
    The child does not return to the shell code
    The child becomes the new program entirely
So the child does not come back to your shell code.
    execvp() destroys the child’s old code and loads new code in its place.

🔥 2. So how does the shell “get back control”?
Not through the child returning.
Not through execvp() returning.

The truth:

The parent is just waiting in waitpid(), blocked.
When the child finishes (exits), the kernel wakes the parent.

Parent code:

pid_t pid = fork();

if (pid == 0) {
    // Child: becomes new program via execvp()
    execvp(argv[0], argv);
    // Only runs if exec failed
    perror("execvp");
    _exit(127);
} else {
    // PARENT: continues running here
    int status;
    waitpid(pid, &status, 0);
    // When waitpid returns → child has EXITED
}

The parent never stops being your shell.
The parent is only sleeping inside waitpid().
When the child exits, the kernel wakes the parent and lets it return from waitpid().
This is how the shell regains control.

🧬 3. What exactly happens inside the kernel?
Let’s do a deep trace:
When you call fork():
    Kernel duplicates the current process into a second process.
    Two processes now exist:
        Parent PID = e.g. 1000
        Child PID = e.g. 1001
In the child:
    You run execvp("ls", ...)
    Kernel:
        Throws away old code (shell code)
        Loads /bin/ls into memory
        Sets instruction pointer to _start of ls
ls now runs as if it was always PID 1001.
There is no link back to the shell code anymore.
ls eventually calls:
exit(0);

Now the kernel:
    Marks the child process as zombie
        (means: "finished, waiting for parent to read exit status")

    Sends a SIGCHLD signal to parent (unless ignored)
    Checks if the parent is blocked in waitpid(pid, ...)
        If yes → kernel wakes up the parent
        Parent is ready to continue executing shell code

At that moment, waitpid() returns.
And the shell prints the prompt again.

🌊 4. Important: The child does NOT “return a value” to the parent like a function
People sometimes think:
“exec returns result to parent”
No.
That NEVER happens.
Here’s what really happens:
    Child → becomes ls
    ls exits → kernel stores its exit status in process table
    Parent → calls waitpid
    Kernel → wakes parent & returns child's exit code
This is NOT "returning" in programming sense; it is a process state change.

🧬 Deep Kernel View: What really happens when SIGCHLD is sent?
Inside the Linux kernel:
    Each process has a task_struct.
    When a child exits:
        Its state changes to TASK_ZOMBIE.
        The kernel stores its exit code in task_struct->exit_code.
        Kernel checks child's parent->signal->handlers[SIGCHLD].
        Kernel adds SIGCHLD to parent’s signal queue.
        If parent is sleeping in wait(), kernel wakes it up.
So SIGCHLD is an asynchronous event telling the parent:
“A child changed state, please check it.”

🎯 Why SIGCHLD allows your shell to be asynchronous
Without SIGCHLD:
    If you start a background job, your shell cannot know when it finishes.
    you cannot reap dead processes until the user runs another command.
    Zombies will pile up.
With SIGCHLD:
    The shell gets notified instantly.
    It reaps children automatically.
    It keeps the prompt responsive.
    No zombies exist.
This is exactly how bash, zsh, fish, and all real shells work.




✅ 1. fork() is extremely cheap because of Copy-On-Write (COW)
You are thinking of old UNIX (1970s) where fork() literally copied the entire memory space.
Modern Linux does not copy memory at fork.
Instead:
Parent and child share all memory pages initially
Pages are copied only if they are written to
Usually, after fork(), the child immediately calls execvp(), which replaces memory anyway
So the cost of fork() today is mostly:
Allocating PCB
Copying page tables (small)
Setting up kernel bookkeeping
Meaning:
fork+exec is almost as cheap as creating a fresh process directly.
This is why it's still the best model.



