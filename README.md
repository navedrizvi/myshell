Built-in commands: "cd", "clr", "dir", "environ", "echo", "help", "pause", "quit"

To run myshell-

1. Open terminal in directory, and enter "make".
2. Enter "./myshell" to run the prompt, or in order to batch process a file follow by the optional filename.

The shell runs user commands by using system calls on \*nix platforms.
All external commands (from unix) except the built-in commands support Input/Output (IO) redirection, piping, and parallel (background) execution in myshell.
The following built-in commands support output redirection in myshell- "dir", "environ", "echo", "help"

IO redirection: and writes it to file, or
cmd [args] > outputFileName : command captures the stdout of the program and will write output to file by creating new file named outputFileName
cmd [args] >> outputFileName : command captures the stdout of the program and appends/write output to file by appending to/creating a new file named outputFileName
cmd [args] < inputFileName : command will use inputFileName as the argument provided by user and run the program using inputfile as stdin

Piping:
cmd1 [args] | cmd2 [args] : cmd1 will execute, the result of which will be the input to cmd2

Program environment:
Is a list of environment variables that refer to properties of the running program, these can be listed by using command "environ"

Background execution:
cmd [args] & : cmd will execute as a background process, and the user can input more commands for processing as it executes
