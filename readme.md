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




# Tokenizer Test Suite (README Format)

This document serves as a test plan for validating the core tokenization logic of a custom shell implementation. The tests are categorized by complexity, covering standard functionality, error handling, and complex edge cases.

---

| Test No. | Section | Case (Input Command) | Purpose | Expected Output (Token List / Behavior) |
| :---: | :--- | :--- | :--- | :--- |
| **1.1** | **Basic Splitting** | `ls -l /home` | Simple argument separation. | `ls`, `-l`, `/home` |
| **1.2** | **Basic Splitting** | `     echo     hello       world` | Handles multiple spaces and leading/trailing whitespace. | `echo`, `hello`, `world` |
| **1.3** | **Basic Splitting** | `echo hello         ` | Handles trailing spaces at end of line. | `echo`, `hello` |
| **2.1** | **Quoted Strings** | `echo "hello world"` | Preserves internal spaces as a single token. | `echo`, `hello world` |
| **2.2** | **Quoted Strings** | `echo ""` | Handles an explicitly empty quoted string. | `echo`, `""` |
| **2.3** | **Quoted Strings** | `ab"cd ef"gh` | Concatenation of adjacent quoted and unquoted segments. | `abcd efgh` |
| **2.4** | **Quoted Strings** | `printf "%s\n"` | Quoted string containing special characters. | `printf`, `%s\n` |
| **2.5** | **Quoted Strings** | `echo "$HOME is cool"` | Special characters inside quotes are treated literally by the tokenizer. | `echo`, `$HOME is cool` |
| **3.1** | **Operator Splitting** | `ls | wc -l` | Standard pipe operator. | `ls`, `|`, `wc`, `-l` |
| **3.2** | **Operator Splitting** | `cat < in.txt > out.txt` | Input and output redirections. | `cat`, `<`, `in.txt`, `>`, `out.txt` |
| **3.3** | **Operator Splitting** | `ls>out` | Redirection adjacent to tokens (no space). | `ls`, `>`, `out` |
| **3.4** | **Operator Splitting** | `sleep 10 &` | Background operator. | `sleep`, `10`, `&` |
| **3.5** | **Operator Splitting** | `ls|grep abc|wc -l` | Multiple adjacent operators. | `ls`, `|`, `grep`, `abc`, `|`, `wc`, `-l` |
| **4.1** | **Edge Cases** | `<>` | Two operators adjacent to each other. | `<`, `>` |
| **4.2** | **Edge Cases** | `|` | Operator as the entire command. | `|` |
| **4.3** | **Edge Cases** | `ls "a   b"     c` | Mixing quoted and unquoted spacing/arguments. | `ls`, `a   b`, `c` |
| **4.4** | **Edge Cases** | `""` | Command is an empty quoted string. | `""` |
| **4.5** | **Edge Cases** | `"hello world">out` | Quoted string immediately followed by an operator. | `hello world`, `>`, `out` |
| **4.6** | **Edge Cases** | `"ab>cd" <"qq" | "rr"&` | Complex mixing of quotes and operators. | `ab>cd`, `<`, `qq`, `|`, `rr`, `&` |
| **4.7** | **Edge Cases** | `&&&&&&&&&&&&&&&&&` | Stress test with repeated operators. | `&`, `&`, `&`, `&`, `&`, `&`, `&`, `&`, `&`, `&`, `&`, `&`, `&`, `&`, `&`, `&`, `&` |
| **5.1** | **Negative/Error** | `"` | Single unmatched quote. | Error: `unmatched quote`, returns `NULL` |
| **5.2** | **Negative/Error** | `echo "hello` | Missing closing quote mid-command. | Error: `unmatched quote`, returns `NULL` |
| **5.3** | **Negative/Error** | `"abc def` | Unclosed quote with internal spaces. | Error: `unmatched quote`, returns `NULL` |
| **6.1** | **Terminal Check** | `^[[A` (Arrow Up) | Terminal control sequence (should be handled by line editor). | No tokens, No segmentation fault |











What this is and why we need it
command_t is the structured command:
argv → what you pass to execvp
infile → if user wrote < file
outfile → if user wrote > file
init_command → makes sure we start from a clean state.
free_command → avoids memory leaks.
parse_tokens → converts a flat list of tokens (["ls","-l",">","out.txt"]) into a structured command_t.



✅ 2. What strdup() does
strdup():
allocates memory on the heap (malloc)
copies the string into that memory
returns the pointer

🔥 Why use strdup() instead of directly assigning?
If you did:
cmd->infile = tokens[i + 1];
Then cmd->infile points to memory inside tokens[].
If tokens gets freed or modified, cmd->infile breaks → dangling pointer.
But with strdup():
You get your own safe copy.
It will remain valid until you free it.