# RainbowShell - Custom Unix Shell

Welcome to **RainbowShell**, my educational but powerful custom Unix shell written in C. RainbowShell implements **pipes**, **redirections**, **logical operators**, **subshells**, and **background jobs** while also featuring a **rainbow/gradient** greeting message (builtes `color.h` library).

> **Note**: This is a project for the 3rd semester at CMC MSU (summarizing skills of systems programming, process handling, shell scripting concepts).

---

## Table of Contents

- [RainbowShell - Custom Unix Shell](#rainbowshell---custom-unix-shell)
  - [Table of Contents](#table-of-contents)
  - [Features](#features)
  - [Build](#build)
  - [Usage](#usage)
  - [Project Structure](#project-structure)
  - [Examples](#examples)
  - [Background Jobs](#background-jobs)

---

## Features

- **Rainbow Greeting**  
  When you launch RainbowShell, you’re greeted with a colorful gradient welcome message. Because, who don't want to meet a rainbow 🌈 ?

- **Custom Command Parser**  
  - Supports multi-stage pipelines: `cmd1 | cmd2 | cmd3`
  - Understands logical operators: `&&`, `||`
  - Sequential execution: `cmd1 ; cmd2`
  - Subshells: `(cd / && ls -l)`
  - Redirections: `< file`, `> file`, `>> file`
  - Background tasks: `cmd &`
  
- **Built-in Commands**  
  - `cd <dir>`: change directory (built-in, because changing directories in a child process has no effect on the parent)

- **Error Handling**  
  - Lexer and parser produce meaningful, color-highlighted error messages (typos, missing quotes, etc).
  - Handles partial command failures in pipelines (ex. if the last command in a pipeline fails, subsequent `&&` sections won’t run).

- **Background Jobs**  
  - Run commands asynchronously with `&`.
  - Automatically ignores `SIGINT` (Ctrl-C) in the background job so it won’t be interrupted.
  - Prevents zombie processes via `waitpid`.

- **Subshell & Grouping**  
  - Parentheses `( ... )` create a new job in a subshell.  
    Example: `(cd /; ls); pwd`
  - You can group multiple commands, run them in the background, or combine with logical operators.

- **Extensible Architecture**  
  - Separation of **lexer**, **parser**, and **executor**.
  - Because of it future features or built-ins can be easily added.

---

## Build 

1. **Clone** this repository:
   ```bash
   git clone https://github.com/estnafinema0/RainbowShell.git
   cd RainbowShell/shell
   ```
2. **Compile** using `make`:
   ```bash
   make
   ```
   Now we have executable `Rainbowshell`.

3. **Run** RainbowShell:
   ```bash
   ./Rainbowshell
   ```

---

## Usage

Now you are open to type any commands as in a standard Unix shell. 
```bash
shell> echo "Hello World"
```

Use **Ctrl-C** to send an interrupt signal to **foreground** commands. If a command is run in the background (using `&`), it won’t be interrupted by **Ctrl-C** (as it should be in a usual shell!).

---

## Project Structure

- **main.c**  
  Entry point for RainbowShell. Handles signals (`SIGINT` and `SIGCHLD`), prints the rainbow greeting, and starts the main interactive loop.

- **parser.c / parser.h**  
  Contains the logic for parsing command lines into a structured form (`Job` and `Command` objects). Handles operators like `&&`, `||`, `|`, `;`, `&`, as well as subshell grouping `( ... )`.

- **lexer.c / lexer.h**  
  Breaks the command line into tokens (`TOKEN_WORD`, `TOKEN_OPERATOR`, etc.). Also handles quoted strings and error checking for unclosed quotes.

- **executor.c / executor.h**  
  Executes commands by forking processes. Implements:
  - Pipelines (`|`)
  - Sequential commands (`;`)
  - Conditional commands (`&&`, `||`)
  - Subshells
  - Redirections (`<`, `>`, `>>`)
  - Background jobs (`&`)

- **builtins.c / builtins.h**  
  Built-in commands such as `cd`.

- **colors.c / colors.h**  
  Provides color/gradient print utilities. The `print_rainbow_text` function produces the signature rainbow greeting.

- **structures.h**  
  Defines the primary data structures (`Command`, `Job`, `OperatorType`) used across the shell.

- **makefile**  
  Contains build rules. Simply run `make`.

---

## Examples

Here are some commands to showcase RainbowShell’s broad functionality:

1. **Subshell & Sequential**  
   ```bash
   (cd /; pwd); pwd
   ```
   - Changes directory to `/`, prints it, then returns to the parent shell and prints its current directory.

2. **Conditional Execution**  
   ```bash
   (false && echo "Skipped 1") || (echo "Fallback 1" && echo "Part of fallback")
   ```
   - `false` fails, so `&&` chain is skipped; `||` triggers the fallback commands.

3. **Background Task with Redirection**  
   ```bash
   echo "Background task" > output.txt & cat output.txt
   ```
   - The `echo` runs in the background, while the shell immediately proceeds to `cat output.txt`.

4. **Pipeline**  
   ```bash
   echo "hello" | grep "hello" && echo "Pipeline succeeded" || echo "Pipeline failed"
   ```
   - If `grep "hello"` finds the text, the `&&` chain continues.

5. **Multiple Steps with Conditionals**  
   ```bash
   ls non_existent_dir && echo "This should not print" || echo "Directory not found"
   ```
   - Fails on the nonexistent directory, triggers the `||` fallback.

6. **Process Listing and Subshell**  
   ```bash
   ps; ls; (cd .. ; ls; ps) && ls && ps
   ```
   - Shows how a subshell can change directory without affecting the parent shell.

7. **Background Sleep**  
   ```bash
   sleep 5 & echo "Background process test"
   ```
   - `sleep` runs in the background, while RainbowShell continues to the next command.

---

## Background Jobs

If you type `&` in the end of the command, it will launch in the background:
- **Non-blocking background**: The shell is immediately ready for the next command.
- **No Zombie Processes**: RainbowShell calls `waitpid` appropriately.
- **Ignored SIGINT**: Background jobs ignor `Ctrl-C` from the main terminal.

---

Thank you for checking out **RainbowShell**! Enjoy the rainbow!🌈 