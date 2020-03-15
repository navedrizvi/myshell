/*TODO = 
mac doesnt have execvpe so change execp in submission - execvpe(argv[0], argv, env) == -1)
*/
//Credits for shell skeleton- Bryant & O'Hallaron pg.790-792

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include "myshell.h"
// #include "utility.c"

#include <dirent.h>

// extern char **environ;

int main(int argc, char *argv[])
{
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
    else if (argc == 2)
    {
        start_batch_command_processing_from_file(argv[1], env);
    }
    else
    {
        perror("Usage: ./myshell [command]");
        exit(1);
    }
}

void edit_environment_shell_path()
{
}

void start_interactive_command_prompt_loop(char *env[])
{
    char user_command[MAXLINE];

    while (1)
    {
        printf("myshell> ");
        Fgets(user_command, MAXLINE, stdin);
        if (feof(stdin))
            exit(0);
        evaluate_command(user_command, env);
    }
}

void start_batch_command_processing_from_file(char *fname, char *env[])
{
    char *line = NULL;
    size_t linecap = 0;
    ssize_t numchars;
    FILE *fp = fopen(fname, "r");

    while ((numchars = getline(&line, &linecap, fp)) > 0)
    {
        evaluate_command(line, env);
    }
}

void evaluate_command(char *user_command, char *env[])
{
    char *argv[MAXARGS];
    char buf[MAXLINE];
    int bg;
    pid_t pid;

    strcpy(buf, user_command);
    bg = parseline(buf, argv); //returns 1 if process should be executed in background (command ends with '&')
    if (argv[0] == NULL)       //ignore empty lines
        return;
    if (!builtin_command(argv, env))
    {
        if ((pid = Fork()) == 0) //child runs user job
        {
            char shell_path[strlen(getenv("PWD") + 6)];
            strcat(shell_path, "shell=");
            strcat(shell_path, getenv("PWD"));
            strcat(shell_path, "/myshell");
            env[0] = shell_path; // Index 0 is the parent path
            if (execvp(argv[0], argv) == -1)
            {
                printf("%s: Command not found\n", argv[0]);
                exit(0);
            }
        }

        if (!bg) //parent waits for foreground job to terminate
        {
            int status;
            if (waitpid(pid, &status, 0) == -1)
                unix_error("waitfg: waitpid error");
        }
        else
            printf("%d %s", pid, user_command);
    }

    return;
}

/* If first arg is builtin command, run it and return true */
int builtin_command(char **argv, char *env[])
{
    if (!strcmp(argv[0], "quit"))
        quit();
    if (!strcmp(argv[0], "dir"))
    {
        dir(getenv("PWD"));
        return 1;
    }
    if (!strcmp(argv[0], "cd"))
    {
        cd(argv[1]);
        return 1;
    }
    if (!strcmp(argv[0], "pause"))
    {
        pause_input();
        return 1;
    }
    if (!strcmp(argv[0], "environ"))
    {
        // list_environ_variables(environ);
        list_environ_variables(env);
        return 1;
    }
    if (!strcmp(argv[0], "echo"))
    {
        echo(argv);
        return 1;
    }
    if (!strcmp(argv[0], "clr"))
    {
        clear_screen();
        return 1;
    }
    // need for help as well
    if (!strcmp(argv[0], "&")) //ignore singleton &
        return 1;
    return 0;
}

int parseline(char *buf, char **argv)
{
    char *delimiter; //points to first space delimiter
    int argc;        //stotres number of args
    int bg;          //is a baground job?

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

    if ((bg = (*argv[argc - 1] == '&')) != 0) //determine if job should run in background
        argv[--argc] = NULL;

    return bg;
}

void unix_error(char *msg)
{
    fprintf(stderr, "%s: %s", msg, strerror(errno));
    exit(EXIT_FAILURE);
}

int Fgets(char *str, int size, FILE *stream)
{
    char *input;
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

// int dir(char *directory)
// {
//     // list contents in directory
//     DIR *directory_pointer;
//     struct dirent *directory_block;
//     directory_pointer = opendir(directory); // opens directory
//     if (directory_pointer == NULL)
//     {
//         print_cmd_error();
//         perror("dir error: could not find directory");
//         return -1;
//     }
//     while ((directory_block = readdir(directory_pointer)) != NULL)
//     {                                            // read directory to block struct
//         printf("%s\t", directory_block->d_name); // print contents to stdout
//     }

//     return 0;
// }

// int cd(char *directory)
// {
//     //issue= you cant cd into by providing absolute path, eg- "\Users\naved\desktop" cant be reached
//     if (chdir(directory) == -1)
//     {
//         print_cmd_error();
//         perror("cd error: could not find directory");

//         return -1;
//     }

//     return 0;
// }

// void print_cmd_error()
// {
//     char error_message[30] = "An error has occured\n";
//     write(STDERR_FILENO, error_message, strlen(error_message));
// }

// void quit()
// {
//     // exit the shell prompt
//     exit(0);
// }

// void pause_input()
// {
//     //pause shell operation until "enter" is pressed
//     printf("Paused: press enter to continue\n");
//     getchar();
// }

// void list_environ_variables(char **env)
// {
//     // prints all environment variables by looping through an array
//     while (*env != NULL)
//         printf("%s\n", *env++);
// }

// void clear_screen()
// {
//     //////////being weird
//     //terminal gets cleared with cursor positioned at the beginning
//     printf("\033[H\033[J");
// }

// void help()
// {
// }