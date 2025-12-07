📘 1. What is a Parser?
In programming languages, compilers, shells, and interpreters, the input goes through two major steps:

Step 1 — Lexical Analysis (Tokenizer)
This takes raw text:
cat < input.txt > output.txt
and breaks it into tokens:

"cat"
"<"
"input.txt"
">"
"output.txt"


The tokenizer only knows:
✔ words
✔ operators
✔ strings
❌ but nothing about meaning or structure


Step 2 — Parsing
A parser reads the list of tokens and understands the structure:
What is the command?
What are the arguments?
Is there input redirection?
Is there output redirection?
Is the syntax valid?
So:
Parsing = understanding and structuring the meaning of tokens.
The output of a parser is usually a structured object, not just strings.



⚙️ 8. What Happens After Parsing?
Your execution step will now:
Fork a child
If cmd.infile != NULL
→ open input file
→ dup2(fd, STDIN_FILENO)

If cmd.outfile != NULL
→ open output file
→ dup2(fd, STDOUT_FILENO)

Call execvp(cmd.argv[0], cmd.argv)
So parser is essential before execution.


⭐ PART 2 — What are File Descriptors?
Every process (program) in Linux automatically starts with:
FD Number	Meaning	Default
0	Standard Input	keyboard
1	Standard Output	terminal
2	Standard Error	terminal


When you type:
cat
It reads from FD 0 (keyboard)
and writes to FD 1 (terminal).
When you type:
ls
It writes to FD 1 (the terminal).


⭐ PART 3 — What Redirection Means
Command:
cat < input.txt
means:
DO NOT read from keyboard
INSTEAD read from input.txt
Command:
ls > output.txt
means:
DO NOT write to terminal
INSTEAD write to output.txt
So the shell must:
Replace FD 0 with input.txt
OR
Replace FD 1 with output.txt
This “replacement” is done with dup2.

⭐ PART 4 — What Does dup2(old_fd, new_fd) Mean?
Let’s say you open a file:
int fd = open("input.txt", O_RDONLY);
Now fd might be something like 3.
Diagram:
FD 0 → keyboard
FD 1 → terminal
FD 2 → terminal
FD 3 → input.txt   <-- new file we opened

Now we call:
dup2(fd, STDIN_FILENO);  // dup2(3, 0)
This means:
Replace FD 0 with FD 3
So now:
FD 0 → input.txt
Cat read from FD 0, so now:
cat reads from input.txt instead of the keyboard

⭐ PART 5 — Visual Diagram of dup2 (VERY IMPORTANT)
Before:
 FD number | points to
-----------+--------------------
     0     | keyboard
     1     | screen
     2     | screen
     3     | input.txt
After:
dup2(3, 0)
New table:
 FD number | points to
-----------+--------------------
     0     | input.txt   <-- changed
     1     | screen
     2     | screen
     3     | input.txt
Now when the program calls:
read(0, ...)
it actually reads the file.



int open(const char *pathname, int flags);

It opens a file and returns a number called a file descriptor (fd).
If success → returns a non-negative integer (3, 4, 5, …)
If error → returns -1

Open the file for reading only.
Other modes include:
O_WRONLY → write-only
O_RDWR → read + write
O_CREAT → create file if it doesn’t exist
O_TRUNC → truncate (clear the file)
But for < input.txt, you only need read mode.


File Descriptor
A file descriptor (FD) is simply an integer that the operating system uses to refer to an open file or input/output resource.
It is a number, nothing more — but it represents something powerful.

Linux automatically creates three file descriptors for every process:
FD Number	Name	Meaning
0	STDIN	Input (keyboard)
1	STDOUT	Output (terminal)
2	STDERR	Error output (terminal)
These are always open.

When you open a new file
The next available FD is used.

Why do shells use file descriptors?
Because of redirection and pipes.

Meaning of dup2(fd, 0) :Make file descriptor fd act as STDIN (0)
dup2(old_fd, new_fd) replaces new_fd with old_fd.


Why do we use it?
When implementing input redirection like:
cat < input.txt
You open the file:
int fd = open("input.txt", O_RDONLY);
Now fd points to the file.
But cat reads from STDIN, not from your file.
So you do:
dup2(fd, 0);
Now:
STDIN (0) points to input.txt
When cat runs, it thinks it is reading from keyboard
But actually reading from the file



How dup2() behaves step-by-step
Suppose:
fd = 3 (because file descriptors 0,1,2 are already taken)
Then before dup2(fd, 0):
FD	Meaning
0	Keyboard
3	input.txt
After calling:
dup2(3, 0);
FD	Meaning
0	input.txt 🟢
3	input.txt
It overwrites FD 0, closes the old STDIN, and duplicates fd into 0.

✔ dup2(fd, 0) = redirect input
✔ dup2(fd, 1) = redirect output
✔ dup2(fd, 2) = redirect error output


We call close(fd) because:
✔ After dup2(fd, STDIN_FILENO) or dup2(fd, STDOUT_FILENO),
the old file descriptor (fd) is no longer needed.
If we don’t close it → your program will leak file descriptors.




1. When is the permission (0644) required?
The permission part is required ONLY when using O_CREAT.
Example:
open(cmd->outfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
Since you are telling the OS “create the file if it doesn’t exist”,
the OS must know what permissions to give that new file.
So:
If the file already exists → permissions are NOT changed
If the file does NOT exist → OS uses 0644 to set permissions
IF you include O_CREAT → open() MUST receive a 3rd argument (mode)


✔ The permission (0644) is required when using O_CREAT
✔ It decides default permissions for new files
✔ It is ignored if the file already exists
❌ You cannot remove it—but you can use a default value

What exactly does 0644 mean?  
It is an octal (base-8) value representing file permissions:
Owner   Group   Others
 r w -   r - -   r - -
Meaning:
Owner: read + write
Group: read
Others: read
This is a very common safe default for output files.
some more exampple for this : 0666  // rw-rw-rw-     0600  // rw-------    0755  // rwxr-xr-x







