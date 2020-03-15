#define MAXLINE 8192
#define MAXARGS 128

void unix_error(char *msg);
void evaluate_command(char *user_command, char *env[]);
void str_concat(char *str1, char *str2);
void start_interactive_command_prompt_loop(char *env[]);
void start_batch_command_processing_from_file(char *fname, char *env[]);
void print_cmd_error();
int builtin_command(char **argv, char *env[]);

int cd(char *directory);
int dir(char *directory);
void quit();
void pause_input();
// void list_environ_variables(char **environment);
void list_environ_variables(char **env);
void echo(char **argv);
void clear_screen();
