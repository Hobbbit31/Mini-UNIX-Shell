
# Mini Shell – User Documentation

## 1. Problem Statement
Design and implement a UNIX-style command-line shell capable of executing external programs, managing processes, handling I/O redirection, pipelines, and basic job control, without invoking another shell.

---
## 2. Introduction
The Mini Shell is a simplified UNIX-like command-line interpreter that demonstrates key operating system concepts such as process creation, command parsing, input/output redirection, pipelines, background execution, and signal handling.  
It displays your current working directory as the shell prompt and waits for user commands in a loop.

---

## 3. Starting the Shell

### Compilation
```
make
```

### Running
```
./mini-shell
```

### Prompt Example
```
/home/hobbbit31/Desktop/mini-shell $
```

---

## 4. Feature Overview
The Mini Shell supports:

- Executing external commands  
- Built-in commands (`cd`, `pwd`, `exit`, `history`)  
- Background execution (`&`)  
- Redirection (`<`, `>`, `>>`)  
- Quoted strings (single & double quotes)  
- Single pipeline support (`|`)  
- Ctrl+C is not terminating the shell 
- Zombie process cleanup 

---

## 5. Executing External Commands
Any program available in the system PATH can be executed.

Examples:
```
ls -l
echo Hello
cat notes.txt
```

---

## 6. Built-in Commands

### 6.1 cd – Change Directory
Usage:
```
cd <directory>
cd ~
cd
```

### 6.2 pwd – Print Working Directory
Displays the absolute path of the current directory.
```
pwd
```

### 6.3 exit – Quit the Shell
Ends the shell session:
```
exit
```

---

## 7. Background Execution (&)
Use `&` to run a command without blocking the shell.

Example:
```
sleep 5 &
```

---

## 8. Input/Output Redirection

### 8.1 Input Redirection (<)
```
grep error < log.txt
```

### 8.2 Output Redirection (>)
```
ls > files.txt
```

### 8.3 Append Output (>>)
```
echo done >> progress.log
```

---

## 9. Quoted Strings
```
echo "Hello World"
grep 'user not found' auth.log
```

---

## 10. Pipelines (|)
```
ls | wc -l
```

---

## 11. Ctrl+C Handling
Pressing Ctrl+C interrupts only the running child process, not the shell itself.

---

## 12. Zombie Process Cleanup
The Mini Shell uses a SIGCHLD handler to automatically reap terminated background processes. When a child process completes sigchld signal on partiular signal one function run which cleans it up.

---

## 13. Error Handling
The shell reports:
- Missing filenames after redirection  
- Misuse of `&`  
- Multiple redirections  
- Incorrect pipeline usage  
- Unmatched quotes  

---

## 14. Usage Examples

### 14.1 Simple
```
ls -l
echo "Mini Shell"
```

### 14.2 Redirection
```
sort nums.txt > sorted.txt
grep hello < input.txt >> output.log
```

### 14.3 Pipelines
```
ls | wc -l

```

### 14.4 Combined
```
cat input.txt | grep pass > result.txt &

```

---

## 15. Exiting the Shell
```
exit