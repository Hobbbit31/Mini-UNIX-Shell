1. What is a zombie process?
When a child process finishes, the OS does not immediately delete it.
Instead, it keeps a small entry in the process table with:
PID
Exit status
Some info for the parent to collect
In this state, the child is:
“terminated but not yet reaped” → this is called a zombie.
Why?
Because the parent is supposed to call:
wait()  or  waitpid()
to collect the child’s exit status and let the OS free the entry.
If the parent never calls wait/waitpid, zombie processes accumulate and can exhaust the process table.
In your shell:
Foreground children are reaped when you do waitpid(child_pid, ...) in your run/execute functions.
Background children (started with &) are not waited on immediately.
→ they will become zombies unless you do something.
That “something” is a SIGCHLD handler.

2. What is SIGCHLD?
SIGCHLD is a signal sent to a process (your shell) when:
one of its child processes terminates
or is stopped (e.g. by Ctrl+Z)
or is continued (if you track that)
Your shell is the parent of all the commands you run (fork() + execvp()).
Whenever a command finishes, the kernel sends your shell:
SIGCHLD
If you do nothing, zombies remain.
If you install a handler for SIGCHLD, you can reap them immediately.
So: SIGCHLD handler = automatic cleaner of dead children.


3. waitpid() with WNOHANG: non-blocking reap
Normally, waitpid waits until a child dies:
waitpid(-1, &status, 0);   // blocking
But inside a signal handler, you must not block, or your shell will freeze.
So you use:
waitpid(-1, &status, WNOHANG);
-1 → “any child process”
&status → where exit status goes (can be NULL)
WNOHANG → do not block; just check briefly
Return values:
> 0 → a child was reaped; return value = child PID
0 → there is at least one child, but none has exited yet
< 0 → error (usually ECHILD → no children left)
Inside the SIGCHLD handler, you want to reap ALL dead children.
So you call waitpid in a loop until there are no more:
while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
    // reaped one child
}



4. Installing a SIGCHLD handler with sigaction
You don’t use the old signal() API for serious shell work.
You use sigaction.
Header:
#include <signal.h>
Struct:
struct sigaction sa;
Important fields:
sa.sa_handler → pointer to your handler function
sa.sa_mask → signal mask to block while handler is running
sa.sa_flags → options (like SA_RESTART)
Example setup:
void sigchld_handler(int sig) {
    // we’ll define this later
}
void install_sigchld_handler(void) {
    struct sigaction sa;
    sigemptyset(&sa.sa_mask);       // no extra signals blocked inside handler
    sa.sa_handler = sigchld_handler;
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    // SA_RESTART: restart some interrupted syscalls (optional but common)
    // SA_NOCLDSTOP: don’t send SIGCHLD when children stop, only when they terminate

    if (sigaction(SIGCHLD, &sa, NULL) < 0) {
        perror("sigaction");
        exit(1);
    }
}
You call install_sigchld_handler() once, before your shell loop.


8. How this connects to background processes (&)
Right now, your shell likely does:
Foreground:
run_command(cmd, background = 0)
After forking, parent calls waitpid(child_pid, ...)
Background:
run_command(cmd, background = 1)
Parent does not wait
Prompt returns immediately
Child runs in background
Without a SIGCHLD handler:
When background child finishes, it becomes a zombie and stays there.
With your SIGCHLD handler:
OS sends SIGCHLD to your shell.
Handler runs, waitpid(-1, &status, WNOHANG) reaps that child.
Zombie is removed.
Your shell continues cleanly.
The handler may also reap foreground children that happen to die between fork and waitpid in your main code — but for a simple shell, this is usually still okay, or you structure the code so main waitpid happens promptly.




another way to do this




What signal() is
✔ How to install a handler
✔ How signals are delivered
✔ What happens behind the scenes
✔ What functions are safe/unsafe
✔ When to use signal() vs sigaction()



1. What is signal()?
signal() is an old Unix function that lets you tell the OS:
“When this signal happens, please run this function (my handler).”
Example:
signal(SIGCHLD, handle_sigchld);
This means:
When ANY child process ends
The OS sends SIGCHLD to your shell
Your function handle_sigchld() is automatically executed


⭐ 2. How the OS delivers a signal
Think of signals like interrupts:
Your program is running normally
A child process ends → kernel generates SIGCHLD
Kernel momentarily stops your program
Kernel calls your handler function
Handler finishes
Kernel resumes your program exactly where it paused
This is why handlers must be very small and fast.


⭐ 3. How signal() installs a handler
When you write:
signal(SIGCHLD, handle_sigchld);
you are telling the OS:
For signal number SIGCHLD
When delivered, call function handle_sigchld()
You are not calling the function immediately.
You are registering it

⭐ 4. What happens when a SIGCHLD actually fires?
Imagine this code:
void handle_sigchld(int sig) {
    while (waitpid(-1, NULL, WNOHANG) > 0);
}
If a child dies:
OS pauses your shell
OS jumps into handle_sigchld
waitpid reaps zombie children
Handler finishes
OS returns to exactly where program was (maybe inside fgets, printf, etc.)


⭐ 5. What can trigger signals
▶ SIGCHLD — child exits
▶ SIGINT — Ctrl + C
▶ SIGTSTP — Ctrl + Z
▶ SIGALRM — alarm() timer
▶ SIGPIPE — writing to broken pipe
▶ SIGTERM — politely asking process to quit
Your shell usually handles:
SIGCHLD manually
SIGINT (often ignored in the shell itself)


⭐ 7. Why do we use WNOHANG inside handler?
Because a signal handler must NEVER block.
OK:
waitpid(-1, &status, WNOHANG);
NOT OK:
waitpid(-1, &status, 0);  // blocks → freezes shell


⭐ 8. Why do we use a loop while(waitpid(...) > 0)?
Because multiple children may die at the same time.
Without the loop → zombies remain.


⭐ Your Question
“signal is raised only when the child process is over, so if it is over why blocking happens? Why do we need WNOHANG?”
This FEELS logical…
but the important thing you’re missing is:
When SIGCHLD arrives, the child is dead, YES — but waitpid(-1, 0) may still block because OTHER children are still alive.
Let’s break it down proper





