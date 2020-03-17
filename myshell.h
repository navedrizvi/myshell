#define MAXLINE 8192
#define MAXARGS 50
#define BUF_SIZE 8000
#define READ 0
#define WRITE 1

void unix_error(char *msg);
void evaluate_command(int argc, char **argv, char *env[]);
void str_concat(char *str1, char *str2);
void start_interactive_command_prompt_loop(char *env[]);
void start_batch_command_processing_from_file(char *fname, char *env[]);
void print_cmd_error();
int is_builtin_command_then_execute(char **argv, char *env[]);
int change_current_directory(char *directory);
int list_directory_contents(char *directory);
void quit();
void pause_input();
void list_environ_variables(char **environment);
void echo(char **argv);
void clear_screen();
void perform_output_redirection_external(char **argv, int index_of_symbol);
char **get_relevant_user_arguments_with_command(char **argv, int index_of_symbol);
int get_valid_output_append_redirection_index(char **argv);
void perform_output_append_redirection_external(char **argv, int index_of_symbol);
void perform_input_redirection_external(char **argv, int index_of_symbol);
int get_valid_input_redirection_index(char **argv);
int get_valid_pipe_symbol_index(char **argv);
void perform_input_and_output_redirection_external(char **argv, int index_of_input_redir_symbol, int index_of_output_redir_symbol);
void perform_input_and_output_append_redirection_external(char **argv, int index_of_input_redir_symbol, int index_of_output_append_redir_symbol);
void perform_pipe_external(int argc, char **argv, char *env[], int index_of_pipe_symbol);
char **get_latter_user_command(int argc, char **argv, int index_of_pipe_symbol);

// evaluate_command(index_of_pipe_symbol, relevant_user_arguments_with_command, env);