
# Mini Shell – Detailed User Documentation

## 1. Introduction
The Mini Shell is a simplified UNIX-like command-line interpreter that demonstrates key operating system concepts such as process creation, command parsing, input/output redirection, pipelines, background execution, and signal handling.  
It displays your current working directory as the shell prompt and waits for user commands in a loop.

---

## 2. Starting the Shell

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
/home/user $
```

---

## 3. Feature Overview
The Mini Shell supports:

- Executing external commands  
- Built-in commands (`cd`, `pwd`, `exit`)  
- Background execution (`&`)  
- Redirection (`<`, `>`, `>>`)  
- Quoted strings (single & double quotes)  
- Single pipeline support (`|`)  
- Ctrl+C custom handling  
- Automatic zombie process cleanup  

---

## 4. Executing External Commands
Any program available in the system PATH can be executed.

Examples:
```
ls -l
echo Hello
cat notes.txt
```

---

## 5. Built-in Commands

### 5.1 cd – Change Directory
Usage:
```
cd <directory>
cd ~
cd
```

### 5.2 pwd – Print Working Directory
Displays the absolute path of the current directory.
```
pwd
```

### 5.3 exit – Quit the Shell
Ends the shell session:
```
exit
```

---

## 6. Background Execution (&)
Use `&` to run a command without blocking the shell.

Example:
```
sleep 5 &
```

---

## 7. Input/Output Redirection

### 7.1 Input Redirection (<)
```
grep error < log.txt
```

### 7.2 Output Redirection (>)
```
ls > files.txt
```

### 7.3 Append Output (>>)
```
echo done >> progress.log
```

---

## 8. Quoted Strings
```
echo "Hello World"
grep 'user not found' auth.log
```

---

## 9. Pipelines (|)
```
ls | wc -l
```

---

## 10. Ctrl+C Handling
Pressing Ctrl+C interrupts only the running child process, not the shell itself.

---

## 11. Automatic Zombie Process Cleanup
The Mini Shell uses a SIGCHLD handler to automatically reap terminated background processes.

---

## 12. Error Handling
The shell reports:
- Missing filenames after redirection  
- Misuse of `&`  
- Multiple redirections  
- Incorrect pipeline usage  
- Unmatched quotes  

---

## 13. Usage Examples

### 13.1 Simple
```
ls -l
echo "Mini Shell"
```

### 13.2 Redirection
```
sort nums.txt > sorted.txt
grep hello < input.txt >> output.log
```

### 13.3 Pipelines
```
dmesg | grep usb
```

### 13.4 Background
```
python script.py &
```

### 13.5 Combined
```
cat input.txt | grep pass > result.txt &
```

---

## 14. Exiting the Shell
```
exit
```
