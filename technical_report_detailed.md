
# Mini Shell – Detailed Technical Report

## 1. Introduction
This technical report explains the internal design, algorithms, and system programming concepts used to build the Mini Shell.  
The project provides a simplified but functionally meaningful model of a UNIX command shell, demonstrating:

- Tokenization and parsing  
- Execution of programs using `fork()` and `execvp()`  
- Input/output redirection  
- Pipeline execution using pipes  
- Background process handling  
- Signal handling (`SIGINT`, `SIGCHLD`)  
- Memory management and error recovery  

The Mini Shell is implemented in modular C source files:  
- `mini-shell.c` – main loop, prompt, built-ins, signal handling  
- `tokenizer.c` – lexical analyzer  
- `parser.c` – syntax analyzer, command struct builder  
- `execute.c` – execution engine  

This structure closely follows the architecture of real shells like `bash`, though in a simplified form.

---

## 2. System Architecture Overview
The Mini Shell follows a traditional shell pipeline:

```
User Input → Tokenizer → Parser → Command Structure → Executor → OS Kernel
```

Each stage is implemented as an independent module.

### 2.1 Tokenizer
The tokenizer is responsible for converting a raw input line into meaningful tokens.  
It handles:

- Words  
- Operators: `<`, `>`, `>>`, `|`, `&`  
- Quoted strings: `"..."`, `'...'`  
- Whitespace compression  

Quoted strings are treated as single tokens, preventing accidental splitting of text containing spaces.

---

## 3. Command Parsing
The parser takes the list of tokens and constructs the internal command representation `command_t`.

### 3.1 Responsibilities of the Parser
The parser:

1. Validates token order and syntax  
2. Identifies redirection operators and associates them with filenames  
3. Detects background operator `&`  
4. Identifies pipeline operator `|`  
5. Builds the `argv[]` array dynamically  
6. Fills the command structure:
   - `argv`
   - `infile`
   - `outfile`
   - `append`
   - `background`

### 3.2 Error Handling in Parsing
The parser detects several common shell errors:

- Missing filenames after redirection operators  
- Multiple input/output redirections  
- Misplaced `&` (must be the last token)  
- Pipeline operator at the beginning or end (`| cmd` or `cmd |`)  
- Multiple pipelines (unsupported)  
- Unmatched quotes (detected during tokenization)

---

## 4. Command Execution Engine
The execution module handles launching commands using process management system calls.

### 4.1 Execution of Single Commands
The following steps occur:

1. The shell calls `fork()` to create a child process  
2. The child process:
   - Restores default `SIGINT` handling  
   - Sets up redirections using `open()` and `dup2()`  
   - Calls `execvp()` to replace itself  
3. The parent process:
   - If the command is foreground: waits using `waitpid()`  
   - If background: prints the PID and does **not** wait  

### 4.2 Input/Output Redirection Handling
- `<` uses `open(infile, O_RDONLY)`  
- `>` uses `open(outfile, O_WRONLY | O_CREAT | O_TRUNC)`  
- `>>` uses `O_APPEND`  

The `dup2()` system call redirects:

```
stdin  → infile
stdout → outfile
```

---

## 5. Pipeline Execution
Pipelines allow output of one command to be used as input for another:

```
cmd1 | cmd2
```

### 5.1 Steps for Pipeline Execution
1. Create a pipe: `pipe(pipefd)`  
2. Fork left command:
   - Redirect its stdout to pipe write end  
3. Fork right command:
   - Redirect its stdin to pipe read end  
4. Parent closes pipe ends  
5. Parent waits for both children  

### 5.2 Limitations
- Only one `|` is supported  
- Multi-stage pipelines (`cmd1 | cmd2 | cmd3`) are not implemented  

This keeps pipeline execution simpler and easier to trace.

---

## 6. Signal Handling
Two major signals are implemented: `SIGINT` and `SIGCHLD`.

### 6.1 SIGINT (Ctrl+C)
- The shell itself must not terminate  
- Only the running child should be interrupted  

Thus:
- Shell installs a custom `SIGINT` handler  
- Child processes reset handler to default behavior  

This matches typical shell behavior.

---

### 6.2 SIGCHLD — Zombie Process Cleanup
Background processes become zombies when they exit.  
To prevent this, the shell installs:

```
signal(SIGCHLD, handle_zoombie);
```

Inside the handler:
```
waitpid(-1, NULL, WNOHANG)
```

This ensures all terminated child processes are cleaned up automatically.

---

## 7. Memory Management
Dynamic memory allocation occurs during:

- Token creation  
- Building `argv[]`  
- Copying filenames for redirection  

To prevent memory leaks, the shell uses:

- `free_tokens()` – frees tokens  
- `free_memory_cmd()` – frees command structures  

All memory is released at the end of each shell loop iteration.

---

## 8. Design Choices and Simplifications

| Feature | Support |
|--------|---------|
| Single background operator | Supported |
| Single pipeline | Supported |
| Basic redirection | Supported |

These simplifications keep the implementation educational, clear, and maintainable.

---

## 9. Testing and Verification

### 9.1 Basic Commands
```
ls
pwd
echo "hello"
```

### 9.2 Redirection
```
echo test > out.txt
grep text < input.txt >> log.txt
```

### 9.3 Pipelines
```
ls | wc -l
cat file.txt | grep hello
```

### 9.4 Background Jobs
```
sleep 5 &
```

### 9.5 Error Handling Tests
- `>`  
- `cmd & arg`  
- `| cmd`  
- Unmatched `"quoted string`  
- Missing filename after `<` or `>`  

The shell correctly reports errors.

---

## 10. Conclusion
The Mini Shell provides a modular, clear implementation of a basic UNIX shell. It demonstrates how shells use system calls such as:

- `fork()`  
- `execvp()`  
- `dup2()`  
- `pipe()`  
- `waitpid()`  
- `signal()`  

It serves as a strong learning tool for understanding operating system fundamentals, including process management, interprocess communication, and signal handling.

---
