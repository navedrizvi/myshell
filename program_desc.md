## Introduction

myshell is a shell which runs user commands by using system calls on \*nix platforms. The shell can process commands in an interactive prompt or from a file

## Program design

Global variables and function headers are contained in "myshell.h"
Built-in functions are implemented in "utility.c"
Main logic of the shell is contained in "myshell.c"

I started by implementing the internal commands in "utility.c", and then implemented the overall logic for command execution of shell in myshell.c. The execution of external commands works by using fork() and execvp() system calls.
Then I implemented the logic for I/O redirection for external commands, after which I did the same for internal commands. After this I implemented the pipe feature. Most of the functions in the program have comments associated, however I rely more on descriptive variable naming which makes the variable names quite long but in my view it makes programs more readable and explicitly express intent.

### Flow-

- main(): decides weather the user wants batch processing, or interactive command prompt and checks valid program invocation.

- start_interactive_command_prompt_loop(): starts command prompt, captures user command, parses the user command into an array, detects if command has a pipe, and evaluates the command accordingly.

- start_batch_command_processing_from_file(file): opens file and evaluates the command line by line.

- evaluate_command(command): checks weather command is a built in command; if so runs the built in command (which are defined in utility) by calling is_builtin_command_then_execute(command), if not then it forks to create a child process which checks if command has IO redirection and makes call to execvp() accordingly using helper method, and the parent waits for this child process to finish before accepting more commands. Also, it checks if the command is a background process, if it is then prompt continues by asking for user input and create more child processes until the background process completes (thus parent process does not call waitpid in this case).

- parseline(command): parses the user command into a string array by ignoring spaces and newlines

- is_builtin_command_then_execute(command): checks if command is built in, chooses the appropriate command and executes the built in command. 4 built in commands support output redirection (dir, environ, echo and help), so each call also checks if there is a output redirection symbol (> or >>) present in the command and executes accordingly.

- Output redirection commands write the stdout of the program to a file specified after the symbol

- Input redirection commands read the stdin of the program from the file specified after the symbol

- Commands with valid piping have 2 commands, first a command which writes to stdout, and second a command which reads from stdin. The piping symbol redirects the output from first command to be the input for the second command by using file descriptors and the pipe() system call. Since child processes share file descriptor of the parent process, the file descriptors serve as a form of Inter Process Communication thus allowing piping of the commands. One process writes to the write section of the piped file descriptor array by changing the stdout to an index in the array (usually index 1) which is done by calling dup2(fd[WRITE], stdout) and executing the first command; the second process reads from the read side by changing its stdin to the read side of the file descriptor array by calling dup2(fd[READ], stdin) and executes the second command with the input provided by the first command, thus writing the results to the default stout (usually the screen unless output redirection is performed in order to write to a specified file).

## Testing plan

Since the overall design of the program was an iterative process, I ran tests while building each of the built-in commands and compared the results to the original Zsh results. Once the output was same I moved to the next step. The test commands were from the assignment specs, as well as examples from the internet mainly while implementing I/O redirection and piping features in order to ensure the commands and arguments are being parsed and executed properly.
