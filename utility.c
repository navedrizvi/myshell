#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

void print_cmd_error()
{
    char error_message[30] = "An error has occured\n";
    write(STDERR_FILENO, error_message, strlen(error_message));
}

int cd(char *directory)
{
    //issue= you cant cd into by providing absolute path, eg- "\Users\naved\desktop" cant be reached
    if (chdir(directory) == -1)
    {
        print_cmd_error();
        perror("cd error: could not find directory");

        return -1;
    }

    return 0;
}

void quit()
{
    // exit the shell prompt
    exit(0);
}

void pause_input()
{
    //pause shell operation until "enter" is pressed
    printf("Paused: press Enter key to continue\n");
    getchar();
}

void list_environ_variables(char **env)
{
    // prints all environment variables by looping through an array
    while (*env != NULL)
        printf("%s\n", *env++);
}

void echo(char **arguments)
{
    //print user comment and newline (multiple spaces/tabs reduced to one)
    *arguments++; //skips user command for echo
    while (*arguments != NULL)
        printf("%s ", *arguments++);
    printf("\n");
}

void clear_screen()
{
    //////////being weird
    //terminal gets cleared with cursor positioned at the beginning
    printf("\033[H\033[J");
}

void help()
{
}

int dir(char *directory)
{
    // list contents in directory
    DIR *directory_pointer;
    struct dirent *directory_block;
    directory_pointer = opendir(directory); // opens directory
    if (directory_pointer == NULL)
    {
        print_cmd_error();
        perror("dir error: could not find directory");
        return -1;
    }
    while ((directory_block = readdir(directory_pointer)) != NULL)
    {                                            // read directory to block struct
        printf("%s\t", directory_block->d_name); // print contents to stdout
    }
    printf("\n");

    return 0;
}