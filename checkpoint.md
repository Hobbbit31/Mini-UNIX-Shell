trim 
creation of token
executing basic cmd
cd Directory traversing


Improved tokenizer: quotes & special tokens
Parser: build command structures (argv, infile, outfile, background, pipe)
I/O redirection in child (apply dup2)
Single pipeline (cmd1 | cmd2)
Background jobs (&)
SIGCHLD handler (reap zombies)
SIGINT handler (Ctrl-C forwarding)
Built-in exit, resource cleanup, valgrind