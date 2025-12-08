& background flag 
Why do we need this flag?
Because execution code (where you call fork() and waitpid()) must know:
should I wait for the child? (foreground)
or should I not wait and immediately return to prompt? (background)
The parser is the only place that understands the syntax (&).
Execution layer does not see raw tokens; it only sees the parsed structure → so it needs the flag.

What if you don’t store this flag?
Then:
Execution layer has no idea if this was sleep 10 or sleep 10 &.
It will treat both exactly the same.
So & in user input does nothing.


3️⃣ Parent logic: wait vs don’t wait
Now we’re in run_single / run_pipeline.
Typical foreground logic (no &)
int run_single(command_t *cmd) {
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return -1;
    }

    if (pid == 0) {
        // child
        execvp(cmd->argv[0], cmd->argv);
        perror("execvp");
        _exit(1);
    } else {
        // parent (the shell)
        int status;
        waitpid(pid, &status, 0);   // 🚨 Important for foreground
        return status;
    }
}
For foreground commands, the parent shell waits.
The shell blocks until the command finishes, just like real shells do:
user types ls
shell runs ls
only after ls is done, you see the prompt again.





Great question — cmd1 & | cmd2 looks confusing, but the behavior is very well-defined in real shells like bash, zsh, and dash.
Let’s break it down so you fully understand what happens.
✅ Short Answer
In all normal Unix shells:
cmd1 & | cmd2 is a syntax error.
Because:
cmd1 & ends a complete command (background job).
After a complete command, | (pipe) cannot appear because there is no command on the left side of the pipe.
So the shell prints a syntax error.
Example in Bash:
bash$ cmd1 & | cmd2
bash: syntax error near unexpected token `|'
✅ Why is this a syntax error?
Let’s analyze step-by-step what the shell parser sees.
Tokens:
cmd1   &   |   cmd2
Step 1: Shell sees cmd1 &
This means:
Run cmd1 in background
This command ends here → it’s a complete job
After a command is complete, the grammar says:
Next token must start a new command, not a pipe operator.
Step 2: Shell sees |
A pipe must have a valid command on both sides:
left_command | right_command
But here, the left side of | is empty, because the previous command was already terminated by &.
So the parser complains.
🔥 Why can’t cmd1 & be piped to cmd2?
Because & tells the shell:
This command is done. Put it in background.
After that, the shell cannot continue the same pipeline.
& and | have different roles:
Symbol	Meaning
&	Ends the current command and runs it in background
Once you put cmd1 in background, it’s detached. It cannot be part of a pipeline anymore.
🚫 So what does NOT happen?
❌ cmd1 does NOT pipe to cmd2
Because backgrounding with & separates the command.
❌ cmd1 does NOT run inside a pipe
It is run as a normal background job.
❌ cmd2 does NOT get anything through stdin
Because no pipe was ever created (syntax error stops parsing).
🟢 Correct equivalents you can do
✔ If you meant: "pipe cmd1 to cmd2, but run in background"
Correct syntax:
cmd1 | cmd2 &
Meaning:
pipeline: cmd1 → cmd2
the entire pipeline runs in background
shell prompt returns immediately
✔ If you meant: "run cmd1 in background AND run cmd2 separately"
Use:
cmd1 & cmd2
Equivalent to:
run cmd1 in background
then run cmd2 in foreground
✔ If you meant: "pipe output of background job into cmd2"
This is not possible directly, because pipes require both ends to be created in the same job.
But you CAN do FIFO named pipes:
mkfifo pipefile
cmd1 > pipefile &
cmd2 < pipefile
Or more advanced process substitution (bash only):
cmd2 < <(cmd1 &)
But this is not standard shell behavior.
🎯 Final Answer (simple)
cmd1 & | cmd2 → Syntax Error
Because:
cmd1 & ends the command
| appears without a left command
A pipeline cannot start after &
