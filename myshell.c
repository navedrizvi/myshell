#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include "myshell.h"
#include <dirent.h>
#include <fcntl.h>

extern char **environ;

int main(int argc, char *argv[])
{
    //Create user environment
    char *env[MAXLINE];
    char shell_path[strlen(getenv("PWD") + 6)];
    strcat(shell_path, "shell=");
    strcat(shell_path, getenv("PWD"));
    strcat(shell_path, "/myshell");
    env[0] = shell_path; // Index 0 is the shell path

    if (argc == 1)
    {
        start_interactive_command_prompt_loop(env);
    }
    else if (argc == 2) //When filename is provided as argument
    {
        start_batch_command_processing_from_file(argv[1], env);
    }
    else
    {
        perror("Usage: ./myshell [command]");
        exit(1);
    }
}

void start_interactive_command_prompt_loop(char *env[])
{
    char user_command[MAXLINE];
    char *argv[MAXARGS];
    char buf[MAXLINE];
    int argc;
    pid_t pid;
    while (1)
    {
        printf("myshell> ");
        Fgets(user_command, MAXLINE, stdin);
        if (feof(stdin))
            exit(0);
        strcpy(buf, user_command);
        argc = parseline(buf, argv); //returns 1 if process should be executed in background (command ends with '&')
        int index_of_pipe_symbol = get_valid_pipe_symbol_index(argv);
        if (index_of_pipe_symbol != -1)
        { // if user command has valid pipe
            if ((pid = Fork()) == 0)
            {
                perform_pipe_external(argc, argv, env, index_of_pipe_symbol);
            }
            else
            {
                int status;
                if (waitpid(pid, &status, 0) == -1)
                    unix_error("waitfg: waitpid error");
            }
        }
        else
        { //case of no pipe
            evaluate_command(argc, argv, env);
        }
    }
}

void start_batch_command_processing_from_file(char *fname, char *env[])
{
    char *user_command_line = NULL;
    size_t linecap = 0;
    ssize_t numchars;
    FILE *fp = fopen(fname, "r");

    char *argv[MAXARGS];
    char buf[MAXLINE];
    int argc;
    int pid;
    while ((numchars = getline(&user_command_line, &linecap, fp)) > 0)
    {
        strcpy(buf, user_command_line);
        argc = parseline(buf, argv); //returns 1 if process should be executed in background (command ends with '&')
        int index_of_pipe_symbol = get_valid_pipe_symbol_index(argv);
        if (index_of_pipe_symbol != -1)
        { // if user command has valid pipe
            if ((pid = Fork()) == 0)
            {
                perform_pipe_external(argc, argv, env, index_of_pipe_symbol);
            }
            else
            {
                int status;
                if (waitpid(pid, &status, 0) == -1)
                    unix_error("waitfg: waitpid error");
            }
        }
        else
        { //case of no pipe
            evaluate_command(argc, argv, env);
        }
    }
}

void evaluate_command(int argc, char **argv, char *env[])
{
    pid_t pid;
    int is_background;

    if (argv[0] == NULL) //ignore empty lines
        return;
    if ((is_background = (*argv[argc - 1] == '&')) != 0) //determine if job should run in background
    {
        argv[--argc] = NULL;
    }
    if (is_builtin_command_then_execute(argv, env) == 0) //evaluates to true when its external command
    {
        if ((pid = Fork()) == 0) //child runs the external command
        {
            //Check if IO redirection characters are present
            int index_of_output_redir_symbol = get_valid_output_redirection_index(argv);
            int index_of_output_redir_append_symbol = get_valid_output_append_redirection_index(argv);
            int index_of_input_redir_symbol = get_valid_input_redirection_index(argv);

            char shell_path[strlen(getenv("PWD") + 6)];
            strcat(shell_path, "shell=");
            strcat(shell_path, getenv("PWD"));
            strcat(shell_path, "/myshell");
            env[0] = shell_path; // Index 0 is the parent path

            //Execute appropriate command based on redirection
            if (index_of_input_redir_symbol != -1 && index_of_output_redir_symbol != -1)
            {
                perform_input_and_output_redirection_external(argv, index_of_input_redir_symbol, index_of_output_redir_symbol);
            }
            else if (index_of_input_redir_symbol != -1 && index_of_output_redir_append_symbol != -1)
            {
                perform_input_and_output_append_redirection_external(argv, index_of_input_redir_symbol, index_of_output_redir_append_symbol);
            }
            else
            {
                if (index_of_output_redir_symbol != -1)
                {
                    perform_output_redirection_external(argv, index_of_output_redir_symbol);
                }
                else if (index_of_output_redir_append_symbol != -1)
                {
                    perform_output_append_redirection_external(argv, index_of_output_redir_append_symbol);
                }
                else if (index_of_input_redir_symbol != -1)
                {
                    perform_input_redirection_external(argv, index_of_input_redir_symbol);
                }

                else
                {
                    if (execvp(argv[0], argv) == -1)
                    {
                        unix_error("Command not found");
                    }
                }
            }

            exit(0);
        }

        if (is_background == 0) // if job is not background, parent waits for foreground job to terminate
        {
            int status;
            if (waitpid(pid, &status, 0) == -1)
                unix_error("waitfg: waitpid error");
        }
        else
            printf("%d %s", pid, strcat(argv[0], "\n"));
    }
    return;
}

/* parses user input into argv, returns count of argv */
int parseline(char *buf, char **argv)
{
    char *delimiter; //points to first space delimiter
    int argc;        //stotres number of args

    if (buf[strlen(buf) - 1] == '\n')
    {
        buf[strlen(buf) - 1] = ' '; // replace trailing \n with space
    }
    else
    {
        buf[strlen(buf)] = ' ';
    }

    while (*buf && (*buf == ' ')) //ignore leading spaces
        buf++;

    /* Build argv */
    argc = 0;

    while ((delimiter = strchr(buf, ' ')))
    {
        argv[argc++] = buf;
        *delimiter = '\0';
        buf = delimiter + 1;
        while (*buf && (*buf == ' ')) //ignore spaces
            buf++;
    }

    argv[argc] = NULL;

    if (argc == 0) //ignore blank line
        return 1;

    return argc;
}

/* If first arg is builtin command, run it and return true */
int is_builtin_command_then_execute(char **argv, char *env[])
{
    if (strcmp(argv[0], "dir") == 0)
    {
        int index_of_output_redir_symbol = get_valid_output_redirection_index(argv);
        int index_of_output_append_redir_symbol = get_valid_output_append_redirection_index(argv);
        if (index_of_output_redir_symbol != -1)
        {
            list_directory_contents_output_redirect(argv[2], getenv("PWD"));
        }
        else if (index_of_output_append_redir_symbol != -1)
        {
            list_directory_contents_output_append_redirect(argv[2], getenv("PWD"));
        }
        else
        {
            list_directory_contents(getenv("PWD"));
        }

        return 1;
    }
    if (strcmp(argv[0], "environ") == 0)
    {
        int index_of_output_redir_symbol = get_valid_output_redirection_index(argv);
        int index_of_output_append_redir_symbol = get_valid_output_append_redirection_index(argv);
        if (index_of_output_redir_symbol != -1)
        {
            list_environ_variables_output_redirect(argv[2], environ);
        }
        else if (index_of_output_append_redir_symbol != -1)
        {
            list_environ_variables_output_append_redirect(argv[2], environ);
        }
        else
        {
            list_environ_variables(environ);
        }

        return 1;
    }
    if (strcmp(argv[0], "echo") == 0)
    {
        int index_of_output_redir_symbol = get_valid_output_redirection_index(argv);
        int index_of_output_append_redir_symbol = get_valid_output_append_redirection_index(argv);

        if (index_of_output_redir_symbol != -1)
        {
            char **relevant_user_arguments_with_command = get_relevant_user_arguments_with_command(argv, index_of_output_redir_symbol);
            echo_output_redirect(index_of_output_redir_symbol, argv[index_of_output_redir_symbol + 1], relevant_user_arguments_with_command);
            free(relevant_user_arguments_with_command);
        }
        else if (index_of_output_append_redir_symbol != -1)
        {
            char **relevant_user_arguments_with_command = get_relevant_user_arguments_with_command(argv, index_of_output_append_redir_symbol);
            echo_output_redirect(index_of_output_append_redir_symbol, argv[index_of_output_append_redir_symbol + 1], relevant_user_arguments_with_command);
            free(relevant_user_arguments_with_command);
        }
        else
        {
            echo(argv);
        }

        return 1;
    }
    if (strcmp(argv[0], "help") == 0)
    {
        int index_of_output_redir_symbol = get_valid_output_redirection_index(argv);
        int index_of_output_append_redir_symbol = get_valid_output_append_redirection_index(argv);

        if (index_of_output_redir_symbol != -1)
        {
            char **relevant_user_arguments_with_command = get_relevant_user_arguments_with_command(argv, index_of_output_redir_symbol);
            help_output_redirect(index_of_output_redir_symbol, argv[index_of_output_redir_symbol + 1]);
            free(relevant_user_arguments_with_command);
        }
        else if (index_of_output_append_redir_symbol != -1)
        {
            char **relevant_user_arguments_with_command = get_relevant_user_arguments_with_command(argv, index_of_output_append_redir_symbol);
            help_output_append_redirect(index_of_output_append_redir_symbol, argv[index_of_output_append_redir_symbol + 1]);
            free(relevant_user_arguments_with_command);
        }
        else
        {
            help();
        }

        return 1;
    }
    if (strcmp(argv[0], "quit") == 0)
        quit();
    if (strcmp(argv[0], "cd") == 0)
    {
        change_current_directory(argv[1]);
        return 1;
    }
    if (strcmp(argv[0], "pause") == 0)
    {
        pause_input();
        return 1;
    }

    if (strcmp(argv[0], "clr") == 0)
    {
        clear_screen();
        return 1;
    }
    // need for help as well
    if (strcmp(argv[0], "&") == 0) //ignore singleton &
        return 1;

    return 0; //not a built-in command
}

int get_valid_output_redirection_index(char **argv) // returns index of output redirection symbol if valid
{
    char **ptr = argv;
    char *command = *ptr;
    ptr++; //go to index 1, to skip the user command
    int i = 0;
    while (*ptr != NULL)
    {
        i++;
        if (strcmp(*ptr, ">") == 0)
        {
            if (strcmp(command, "cd") == 0 || strcmp(command, "clr") == 0 || strcmp(command, "pause") == 0 || strcmp(command, "quit") == 0)
            {
                printf("%s is an invalid command for output redirection", command);
                exit(0);
            }
            if (*(ptr + 1) != NULL) //its a valid out redirection since next argument contains output filename
            {
                return i;
            }
            else
            {
                unix_error("Error: invalid out redirection:");
            }
        }
        ++ptr;
    }
    return -1;
}

int get_valid_output_append_redirection_index(char **argv) // returns index of redirection append symbol if valid
{
    char **ptr = argv;
    char *command = *ptr;
    ptr++; //go to index 1, to skip the user command
    int i = 0;
    while (*ptr != NULL)
    {
        i++;
        if (strcmp(*ptr, ">>") == 0)
        {
            if (strcmp(command, "cd") == 0 || strcmp(command, "clr") == 0 || strcmp(command, "pause") == 0 || strcmp(command, "quit") == 0)
            {
                printf("%s is an invalid command for output redirection", command);
                exit(0);
            }
            if (*(ptr + 1) != NULL) //its a valid out redirection since next argument contains output filename
            {
                return i;
            }
            else
            {
                unix_error("Error: invalid out redirection:");
            }
        }
        ++ptr;
    }
    return -1;
}

int get_valid_input_redirection_index(char **argv) // returns index of input redirection symbol if valid
{
    char **ptr = argv;
    char *command = *ptr;
    ptr++; //go to index 1, to skip the user command
    int i = 0;
    while (*ptr != NULL)
    {
        i++;
        if (strcmp(*ptr, "<") == 0)
        {
            if (strcmp(command, "cd") == 0 || strcmp(command, "clr") == 0 || strcmp(command, "pause") == 0 || strcmp(command, "quit") == 0 || strcmp(command, "dir") == 0 || strcmp(command, "environ") == 0 || strcmp(command, "echo") == 0 || strcmp(command, "help") == 0)
            {
                printf("%s is an invalid command for output redirection", command);
                exit(1);
            }
            if (*(ptr + 1) != NULL) //its a valid out redirection since next argument contains output filename
            {
                return i;
            }
            else
            {
                unix_error("Error: invalid out redirection:");
            }
        }
        ++ptr;
    }
    return -1;
}

int get_valid_pipe_symbol_index(char **argv) // returns index of redirection append symbol if valid
{
    char **ptr = argv;
    char *command = *ptr;
    ptr++; //go to index 1, to skip the user command
    int i = 0;
    while (*ptr != NULL)
    {
        i++;
        if (strcmp(*ptr, "|") == 0)
        {
            if (strcmp(command, "cd") == 0 || strcmp(command, "clr") == 0 || strcmp(command, "pause") == 0 || strcmp(command, "quit") == 0 || strcmp(command, "dir") == 0 || strcmp(command, "environ") == 0 || strcmp(command, "echo") == 0 || strcmp(command, "help") == 0)
            {
                printf("%s is an invalid command for output redirection", command);
                exit(1);
            }
            if (*(ptr + 1) != NULL) //its a valid out redirection since next argument contains output filename
            {
                return i;
            }
            else
            {
                unix_error("Error: invalid out redirection:");
            }
        }
        ++ptr;
    }
    return -1;
}

void perform_output_redirection_external(char **argv, int index_of_symbol)
{
    pid_t pid;
    if ((pid = Fork()) == 0) //child executes
    {
        int out_file_fd = open(argv[index_of_symbol + 1], O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO); //O_WRONLY | O_CREAT | O_APPEND in case of appending, O_RDONLY for input redirection
        dup2(out_file_fd, STDOUT_FILENO);                                                                             //the file discriptor represents the stdout now, so result of exec will output to this file                                                                            //replace stdout with newly created file
        close(out_file_fd);
        char **relevant_user_arguments_with_command = get_relevant_user_arguments_with_command(argv, index_of_symbol);
        if (execvp(argv[0], relevant_user_arguments_with_command) == -1)
        {
            unix_error("Execvpe error");
        }
        exit(0);
    }
    else
    {
        //Parent process waits for child to finish
        int status = 0;
        wait(&status);
    }
}

void perform_output_append_redirection_external(char **argv, int index_of_symbol)
{
    pid_t pid;
    if ((pid = Fork()) == 0) //child executes
    {
        int out_file_fd = open(argv[index_of_symbol + 1], O_WRONLY | O_CREAT | O_APPEND, S_IRWXU | S_IRWXG | S_IRWXO); //opens file in append mode, or creates file if doesnt exist
        dup2(out_file_fd, STDOUT_FILENO);                                                                              //the file discriptor represents the stdout now, so result of exec will output to this file                                                                            //replace stdout with newly created file
        close(out_file_fd);
        char **relevant_user_arguments_with_command = get_relevant_user_arguments_with_command(argv, index_of_symbol);
        if (execvp(argv[0], relevant_user_arguments_with_command) == -1)
        {
            unix_error("Execvpe error");
        }
        exit(0);
    }
    else
    {
        //Parent process waits for child to finish
        int status = 0;
        wait(&status);
    }
}

void perform_input_redirection_external(char **argv, int index_of_symbol)
{
    pid_t pid;
    if ((pid = Fork()) == 0) //child executes
    {
        int in_file_fd;
        if ((in_file_fd = open(argv[index_of_symbol + 1], O_RDONLY, S_IRWXU | S_IRWXG | S_IRWXO)) == -1) //opens file in read mode, throws error if file doesnt exist
        {
            unix_error("Input redirection error: error in reading the input file");
        }
        dup2(in_file_fd, STDIN_FILENO); //the file discriptor represents the stdin now, so the file will be stdin for exec                                                                       //replace stdout with newly created file
        close(in_file_fd);
        char **relevant_user_arguments_with_command = get_relevant_user_arguments_with_command(argv, index_of_symbol);
        if (execvp(argv[0], relevant_user_arguments_with_command) == -1)
        {
            unix_error("Execvpe error");
        }
        exit(0);
    }
    else
    {
        //Parent process waits for child to finish
        int status = 0;
        wait(&status);
    }
}

void perform_input_and_output_redirection_external(char **argv, int index_of_input_redir_symbol, int index_of_output_redir_symbol)
{
    pid_t pid;
    if ((pid = Fork()) == 0) //child executes
    {
        int in_file_fd;
        int out_file_fd = open(argv[index_of_output_redir_symbol + 1], O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO); //O_WRONLY | O_CREAT | O_APPEND in case of appending, O_RDONLY for input redirection
        dup2(out_file_fd, STDOUT_FILENO);                                                                                          //the file discriptor represents the stdout now, so result of exec will output to this file                                                                            //replace stdout with newly created file
        close(out_file_fd);
        if ((in_file_fd = open(argv[index_of_input_redir_symbol + 1], O_RDONLY, S_IRWXU | S_IRWXG | S_IRWXO)) == -1) //opens file in read mode, throws error if file doesnt exist
        {
            unix_error("Input redirection error: error in reading the input file");
        }
        dup2(in_file_fd, STDIN_FILENO); //the file discriptor represents the stdin now, so the file will be stdin for exec                                                                       //replace stdout with newly created file
        close(in_file_fd);
        char **relevant_user_arguments_with_command = get_relevant_user_arguments_with_command(argv, index_of_input_redir_symbol);
        if (execvp(argv[0], relevant_user_arguments_with_command) == -1)
        {
            unix_error("Execvpe error");
        }
        exit(0);
    }
    else
    {
        //Parent process waits for child to finish
        int status = 0;
        wait(&status);
    }
}

void perform_input_and_output_append_redirection_external(char **argv, int index_of_input_redir_symbol, int index_of_output_append_redir_symbol)
{
    pid_t pid;
    if ((pid = Fork()) == 0) //child executes
    {
        int in_file_fd;
        int out_file_fd = open(argv[index_of_output_append_redir_symbol + 1], O_WRONLY | O_CREAT | O_APPEND, S_IRWXU | S_IRWXG | S_IRWXO); //opens file in append mode, or creates file if doesnt exist
        dup2(out_file_fd, STDOUT_FILENO);                                                                                                  //the file discriptor represents the stdout now, so result of exec will output to this file                                                                            //replace stdout with newly created file
        close(out_file_fd);
        if ((in_file_fd = open(argv[index_of_input_redir_symbol + 1], O_RDONLY, S_IRWXU | S_IRWXG | S_IRWXO)) == -1) //opens file in read mode, throws error if file doesnt exist
        {
            unix_error("Input redirection error: error in reading the input file");
        }
        dup2(in_file_fd, STDIN_FILENO); //the file discriptor represents the stdin now, so the file will be stdin for exec                                                                       //replace stdout with newly created file
        close(in_file_fd);
        char **relevant_user_arguments_with_command = get_relevant_user_arguments_with_command(argv, index_of_input_redir_symbol);
        if (execvp(argv[0], relevant_user_arguments_with_command) == -1)
        {
            unix_error("Execvpe error");
        }
        exit(0);
    }
    else
    {
        //Parent process waits for child to finish
        int status = 0;
        wait(&status);
    }
}

void perform_pipe_external(int argc, char **argv, char *env[], int index_of_pipe_symbol)
{
    int fd[2];
    pid_t pid;

    if (pipe(fd) == -1)
    {
        unix_error("Pipe error");
    }

    if ((pid = Fork()) == 0) //in child process, command 1 executes, which output is read by parent as STDIN
    {
        if (close(fd[READ]) == -1) //close unused input half of pipe
        {
            unix_error("Error in closing the read side of pipe");
        }
        dup2(fd[WRITE], STDOUT_FILENO);
        close(fd[WRITE]);
        char **relevant_user_arguments_with_command = get_relevant_user_arguments_with_command(argv, index_of_pipe_symbol);
        if (execvp(relevant_user_arguments_with_command[0], relevant_user_arguments_with_command) == -1) //execute command one with relevant arguments
        {
            unix_error("Execvpe error in pipe 1");
        }
    }
    else //in parent process, command 2 executes, with STDIN provided by child
    {
        if (close(fd[WRITE]) == -1) //close unused output half of pipe
        {
            unix_error("Error in closing the write side of pipe");
        }
        dup2(fd[READ], STDIN_FILENO);
        close(fd[READ]);
        char **relevant_user_arguments_with_command = get_latter_user_command(argc, argv, index_of_pipe_symbol);
        if (execvp(relevant_user_arguments_with_command[0], relevant_user_arguments_with_command) == -1) //execute command one with relevant arguments
        {
            unix_error("Execvpe error in pipe 2");
        }
    }

    return;
}

char **get_relevant_user_arguments_with_command(char **argv, int index_of_symbol)
{
    char **output = malloc((index_of_symbol + 1) * sizeof(char *)); //allocate enough space for command

    int out_ind = 0;
    for (int i = 0; i < index_of_symbol; i++)
    {
        output[out_ind] = malloc(strlen(argv[i]) * sizeof(char));
        strcpy(output[out_ind], argv[i]);
        out_ind++;
    }

    return output;
}

char **get_latter_user_command(int argc, char **argv, int index_of_pipe_symbol)
{
    char **output = malloc((argc - index_of_pipe_symbol) * sizeof(char *)); //allocate enough space for latter command

    int out_ind = 0;
    for (int i = index_of_pipe_symbol + 1; i < argc; i++)
    {
        output[out_ind] = malloc(strlen(argv[i]) * sizeof(char));
        strcpy(output[out_ind], argv[i]);
        out_ind++;
    }

    return output;
}

void unix_error(char *msg)
{
    fprintf(stderr, "%s: %s\n", msg, strerror(errno));
    exit(EXIT_FAILURE);
}

int Fgets(char *str, int size, FILE *stream)
{
    if (fgets(str, size, stream) == NULL)
    {
        unix_error("Fgets error");
    }
    return 0;
}

pid_t Fork(void)
{
    pid_t pid;

    if ((pid = fork()) == -1)
    {
        unix_error("Fork error");
    }
    return pid;
}
