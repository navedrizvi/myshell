### To run myshell:

1. Open terminal in directory, and enter "make".
2. Enter "./myshell" to run the prompt, or in order to batch process a file follow by the optional filename.

### Built-in commands:

```
cd [directory] : Changes current working directory to specified directory.
clr : Clears the screen.
dir [directory] : Lists files in the current directory or in specified directory.
environ : Prints current environment variables.
echo [comment] : Prints comment to the screen.
help : Prints the contents of this manual (README.md) using more command.
quit : Exits the myshell program.
```

The shell runs user commands by using system calls on \*nix platforms.
All external commands (from unix) except the built-in commands support Input/Output (IO) redirection, piping, and parallel (background) execution in myshell.
Built-in commands that support output redirection in myshell: "dir", "environ", "echo", "help"

I\O redirection:

```
cmd [args] > outputFileName : command captures the stdout of the program and will write output to file by creating new file, or overwriting existing file named outputFileName
cmd [args] >> outputFileName : command captures the stdout of the program and appends/write output to file by appending to/creating a new file named outputFileName
cmd [args] < inputFileName : command will use inputFileName as the argument provided by user and run the program using inputfile as stdin
```

Piping: multiple programs can be strung together using the "|" (pipe) character

```
cmd1 [args] | cmd2 [args] : cmd1 will execute, the result of which will be the input to cmd2. cmd1 must give output, and cmd2 must take an input for a successful pipe.
```

Background execution:

```
cmd [args] & : cmd will execute as a background process, and the user can input more commands for processing as it executes. There should be a space before "&"
```
