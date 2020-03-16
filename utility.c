#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>

void print_cmd_error()
{
    char error_message[30] = "An error has occured\n";
    write(STDERR_FILENO, error_message, strlen(error_message));
}

int list_directory_contents(char *directory)
{
    // list contents in directory
    DIR *directory_pointer;
    struct dirent *directory_block;
    directory_pointer = opendir(directory); // opens directory
    if (directory_pointer == NULL)
    {
        print_cmd_error();
        printf("dir error: could not find directory '%s'", directory);
        return -1;
    }
    while ((directory_block = readdir(directory_pointer)) != NULL)
    {                                            // read directory to block struct
        printf("%s\t", directory_block->d_name); // print contents to stdout
    }
    printf("\n");

    return 0;
}

int list_directory_contents_output_redirect(char *file_name, char *directory)
{
    DIR *directory_pointer;
    struct dirent *directory_block;
    directory_pointer = opendir(directory); // opens directory
    if (directory_pointer == NULL)
    {
        print_cmd_error();
        printf("dir error: could not find directory '%s'", directory);
        return -1;
    }
    int out_file_fd = open(file_name, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO); //O_WRONLY | O_CREAT | O_APPEND in case of appending, O_RDONLY for input redirection
    while ((directory_block = readdir(directory_pointer)) != NULL)
    { // read directory to block struct
        write(out_file_fd, strcat(directory_block->d_name, "\n"), strlen(directory_block->d_name));
    }
    write(out_file_fd, "\n", 1); // newline for clarity

    return 0;
}

int list_directory_contents_output_append_redirect(char *file_name, char *directory)
{
    DIR *directory_pointer;
    struct dirent *directory_block;
    directory_pointer = opendir(directory); // opens directory
    if (directory_pointer == NULL)
    {
        print_cmd_error();
        printf("dir error: could not find directory '%s'", directory);
        return -1;
    }
    int out_file_fd = open(file_name, O_WRONLY | O_CREAT | O_APPEND, S_IRWXU | S_IRWXG | S_IRWXO); //opens file in append mode, or creates file if doesnt exist
    while ((directory_block = readdir(directory_pointer)) != NULL)
    { // read directory to block struct
        write(out_file_fd, strcat(directory_block->d_name, "\n"), strlen(directory_block->d_name));
    }
    write(out_file_fd, "\n", 1); // newline for clarity

    return 0;
}

void list_environ_variables(char **environment)
{
    // prints all environment variables by looping through the env array
    while (*environment != NULL)
        printf("%s\n", *environment++);
}

void list_environ_variables_output_redirect(char *file_name, char **environment)
{
    // writes all environment variables to a file
    int out_file_fd = open(file_name, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO); //O_WRONLY | O_CREAT | O_APPEND in case of appending, O_RDONLY for input redirection
    while (*environment != NULL)
    {
        write(out_file_fd, *environment, strlen(*environment));
        environment++;
    }
    write(out_file_fd, "\n", 1); // newline for clarity
    close(out_file_fd);
}

void list_environ_variables_output_append_redirect(char *file_name, char **environment)
{
    // appends all environment variables to a file
    int out_file_fd = open(file_name, O_WRONLY | O_CREAT | O_APPEND, S_IRWXU | S_IRWXG | S_IRWXO); //opens file in append mode, or creates file if doesnt exist
    while (*environment != NULL)
    {
        write(out_file_fd, *environment, strlen(*environment));
        environment++;
    }
    write(out_file_fd, "\n", 1); // newline for clarity
    close(out_file_fd);
}

void echo(char **arguments)
{
    //print user comment and newline (multiple spaces/tabs reduced to one)
    *arguments++; //skips user command for echo
    while (*arguments != NULL)
        printf("%s ", *arguments++);
    printf("\n");
}

void echo_output_redirect(int output_redir_symbol_index, char *file_name, char **relevant_arguments)
{
    int out_file_fd = open(file_name, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO); //O_WRONLY | O_CREAT | O_APPEND in case of appending, O_RDONLY for input redirection
    *relevant_arguments++;                                                                        //skips user command for echo
    while (*relevant_arguments != NULL)
    {
        write(out_file_fd, strcat(*relevant_arguments, " "), strlen(*relevant_arguments));
        relevant_arguments++;
    }
    write(out_file_fd, "\n", 1); // newline for clarity
    close(out_file_fd);
}

void echo_output_append_redirect(int output_redir_append_symbol_index, char *file_name, char **relevant_arguments)
{
    int out_file_fd = open(file_name, O_WRONLY | O_CREAT | O_APPEND, S_IRWXU | S_IRWXG | S_IRWXO); //opens file in append mode, or creates file if doesnt exist
    *relevant_arguments++;                                                                         //skips user command for echo
    while (*relevant_arguments != NULL)
    {
        write(out_file_fd, strcat(*relevant_arguments, " "), strlen(*relevant_arguments));
        relevant_arguments++;
    }
    write(out_file_fd, "\n", 1); // newline for clarity
    close(out_file_fd);
}

void help()
{
}

void quit()
{
    // exit the shell prompt
    exit(0);
}

int change_current_directory(char *directory)
{
    if (directory == NULL)
    {
        printf("please provide directory to change to as an argument\n");
        printf("current directory: %s\n", getenv("PWD"));
    }
    if (chdir(directory) == -1)
    {
        print_cmd_error();
        printf("cd error: could not find directory '%s'\n", directory);

        return -1;
    }
    //change PWD environment variable after successful call

    char wd[strlen(getenv("PWD")) + strlen(directory) - 2];
    strcpy(wd, getenv("PWD"));
    strcat(wd, "/");
    strcat(wd, directory);
    setenv("PWD", wd, 1); //resets PWD
    return 0;
}

void pause_input()
{
    //pause shell operation until "enter" is pressed
    printf("Paused: press Enter key to continue\n");
    getchar();
}

void clear_screen()
{
    //////////being weird
    //terminal gets cleared with cursor positioned at the beginning
    printf("\033[H\033[J");
}