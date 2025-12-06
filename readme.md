ctrl+d
In Linux (and Unix-like systems), Ctrl-D is a keyboard shortcut that tells the terminal:
“Send an EOF (End-Of-File) signal to the program reading from standard input.”
Some beginners think when a user "finishes" input by pressing Enter, it means EOF — but that is wrong. Enter simply ends one line, not the entire input.
EOF means end of all input, not end of a line.
"because \n is not EOF.", AS EOF is a Condition Not a Key.

When you run:
fgets(buffer, size, stdin);
it reads from whatever you type in the terminal.

This uses a pipe (|) in Linux.
Connects the output of the first program into the input (stdin) of the second program.

Functions from <ctype.h> like:
isspace()
isdigit()
isalpha()
toupper()
tolower()
these above function are onlt used when the input is unsigned char values.

isspace() :checks for ALL whitespace characters,
isspace(' ')  → true  32 ascii values
isspace('\n') → true  10
isspace('\t') → true  9
isspace('A')  → false
isspace('1')  → false


regarding bash and system call or files
✔️ What IS allowed (and required)
You must directly handle commands using the core system calls:
✔️ fork()
To create a new process.
✔️ execvp()
To replace the child process with the new program binary.
execvp(argv[0], argv);
✔️ waitpid()
To wait for the child to finish (unless background job).
✔️ I/O redirection using dup2()
For < file, > file
✔️ Using pipe()
For cmd1 | cmd2
✔️ Parsing tokens yourself
Using your tokenizer (not bash's logic).
✔️ PATH lookup automatically
Handled inside execvp().


getcwd() simply asks the Linux/Unix kernel:
It is a pure system-level function, not a shell, not a command, not a subprocess.


char *strtok(char *str, const char *delim);

perror() is a C library function used to print error messages that correspond to the current value of errno.


waitpid(pid, &status, 0);
status is an int where the kernel encodes how the child terminated:
did it exit normally, was it killed by a signal, did it core dump, was it stopped, etc.
regarding the last attribute
Flag	Meaning
0	Normal blocking wait
WNOHANG	Non-blocking wait
WUNTRACED	Return if child is stopped
WCONTINUED	Return if child continued
WNOWAIT (Linux specific)	Do not clear child, no zombie removal
