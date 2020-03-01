#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>

#define MAXLINE 8192
#define MAXARGS 128

extern char **environ;

int main()
{
    char cmdline[MAXLINE];

    while (1)
    {

        printf("myshell> ");
        Fgets(cmdline, MAXLINE, stdin);
        if (feof(stdin))
            exit(0);
        printf("%s", cmdline);
        eval(cmdline);
    }
}

void eval(char *cmdline)
{
    char *argv[MAXARGS];
    char buf[MAXLINE];
    int bg;
    pid_t pid;

    strcpy(buf, cmdline);
    bg = parseline(buf, argv); //returns 1 if process should be executed in background (command ends with '&')
    if (argv[0] == NULL)
        return; //Ignore empty lines

    if (!builtin_command(argv))
    {
        if ((pid == Fork()) == 0) //child runs user job
        {
            if (execve(argv[0], argv, environ) == -1)
            {
                printf("%s: Command not found\n", argv[0]);
                exit(0);
            }
        }

        if (!bg) //parent waits for foreground job to terminate
        {
            int status;
            if (waitpid(pid, &status, 0) == -1)
                unix_error("waitfb: waitpid error");
        }
        else
            printf("%d %s", pid, cmdline);
    }

    return;
}

/* If first arg is builtin command, run it and return true */
int builtin_command(char **argv)
{
    if (!strcmp(argv[0], "quit"))
        exit(0);
    if (!strcmp(argv[0], "&")) //ignore singleton &
        return 1;
    return 0;
}

int parseline(char *buf, char **argv)
{
    char *delimiter; //points to first space delimiter
    int argc;        //stotres number of args
    int bg;          //is a baground job?

    buf[strlen(buf) - 1] = ' ';   // replace trailing \n with space
    while (*buf && (*buf == ' ')) //ignore leading spaces
        buf++;

    /* build argv */
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
