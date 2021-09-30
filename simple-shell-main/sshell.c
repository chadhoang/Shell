#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

#define CMDLINE_MAX 512
#define ARG_MAX 16
#define TOKEN_MAX 32
#define FILE_DEST_INDEX 1
#define START_INDEX 0
#define EMPTY_CHAR 0
#define DEFAULT_PERMISSIONS 0644
#define T 1
#define F 0
#define MAX_PIPE_PROCESSES 4
#define READ 0
#define WRITE 1
#define MAX_VARIABLES 26
#define ASCII_CONV 97
#define VAR_INDEX 1

void pwd(char *cmd);
void ex(char *cmd);

struct process {
        char *processes[ARG_MAX];
        int out_redir_call;
        char *out_file;
        int pipe_call;
        int num_pipes;
        char **multi_process[MAX_PIPE_PROCESSES];
};

int parse_cmd(struct process *p, char *cmd, char *set[MAX_VARIABLES]);
void cd(struct process *p, char *cmd);
void reg_command(struct process *p, char *cmd);
void init_process(struct process *p);
void free_process(struct process *p);
void out_redir(struct process *p, char *cmd);
char *arg_instring(struct process *p);
void piping(struct process *p, char *cmd);
void split_processes(struct process *p, int array[MAX_PIPE_PROCESSES]);
void init_multi_processes(struct process *p);
void set_var(struct process *p, char *set[MAX_VARIABLES]);


int main(void)
{
        char cmd[CMDLINE_MAX];
        char *set[MAX_VARIABLES] = { "" };

        while (T) {
                char *nl;
                /* Unused code from original file.
                   int retval;*/

                /* Print prompt */
                printf("sshell@ucd$ ");
                fflush(stdout);

                /* Get command line */
                fgets(cmd, CMDLINE_MAX, stdin);

                /* Print command line if stdin is not provided by terminal */
                if (!isatty(STDIN_FILENO)) {
                        printf("%s", cmd);
                        fflush(stdout);
                }

                /* Remove trailing newline from command line */
                nl = strchr(cmd, '\n');
                if (nl)
                        *nl = '\0';

                /* Parse command line */
                struct process p;
                init_process(&p);
                if (parse_cmd(&p, cmd, set) == 1)
                        continue;

                /* Builtin commands */
                if (!strcmp(cmd, "exit")) {
                        ex(cmd);
                        break;
                } else if (!strcmp(cmd, "pwd")) {
                        pwd(cmd);
                        continue;
                } else if (!strcmp(p.processes[START_INDEX], "cd")) {
                        cd(&p, cmd);
                        continue;
                } else if (!strcmp(p.processes[START_INDEX], "set")) {
                        set_var(&p, set);
                        continue;
                }

                /* Regular commands */
                if (p.pipe_call == T) {
                        piping(&p, cmd);
                } else if (p.out_redir_call == T) {
                        out_redir(&p, cmd);
                } else
                        reg_command(&p, cmd);
        }

        return EXIT_SUCCESS;
}

void pwd(char *cmd)
{
        char cwd[CMDLINE_MAX];

        getcwd(cwd, sizeof(cwd));
        printf("%s\n", cwd);

        int status;
        // pwd skip the fork to change the directory for user
        pid_t pid = 0;

        waitpid(pid, &status, 0);
        fprintf(stderr, "+ completed '%s' [%d]\n",
                cmd, WEXITSTATUS(status));
}

void ex(char *cmd)
{
        fprintf(stderr, "Bye...\n");

        int status;
        pid_t pid;

        pid = fork();
        if (pid == 0) {
                // Child process
                exit(0);
                perror("exit");
        } else if (pid > 0) {
                // Parent process
                waitpid(pid, &status, 0);
                fprintf(stderr, "+ completed '%s' [%d]\n",
                        cmd, WEXITSTATUS(status));
        } else {
                perror("fork");
                exit(1);
        }
}

int parse_cmd(struct process *p, char *cmd, char *set[MAX_VARIABLES])
{
        int process_index = 0;
        int process_char_index = 0;
        int pipes[MAX_PIPE_PROCESSES] = { 0 };
        int pipe_index = 0;
        int var_call = F;
        int entering_space = F;
        int entering_out = F;
        int entering_pipe = F;

        for (int i = 0; i < CMDLINE_MAX; i++) {
                if (process_index >= 16) {
                        fprintf(stderr, "Error: too many process arguments\n");
                        return 1;
                }
                if (cmd[i] == 0) {
                        if (entering_out == T) {
                                fprintf(stderr, "Error: no output file\n");
                                return 1;
                        } else if (entering_pipe == T) {
                                fprintf(stderr, "Error: missing command\n");
                                return 1;
                        }
                        if (entering_space == F && p->out_redir_call == F) {
                                if (p->pipe_call == T)
                                        pipes[pipe_index] = process_index;
                                p->processes[++process_index] = NULL;
                        }else if (entering_space == T || p->out_redir_call == T) {
                                if (p->pipe_call == T)
                                        pipes[pipe_index] = process_index - 1;
                                p->processes[process_index] = NULL;
                        }
                        break;
                } else if (cmd[i] == '>') {
                        if (entering_space == F) {
                                p->processes[process_index++][process_char_index] = '\0';
                                process_char_index = 0;
                                entering_space = T;
                                if (p->processes[process_index - 1][process_char_index] == 0) {
                                        fprintf(stderr, "Error: missing command\n");
                                        return 1;
                                }
                        }
                        p->out_redir_call = T;
                        entering_out = T;
                } else if (cmd[i] == '|') {
                        if (entering_space == F) {
                                p->processes[process_index][process_char_index] = '\0';
                                process_char_index = 0;
                                entering_space = T;
                                pipes[pipe_index++] = process_index;
                                process_index++;
                                if (p->processes[process_index - 1][process_char_index] == 0) {
                                        fprintf(stderr, "Error: missing command\n");
                                        return 1;
                                }
                        } else
                                pipes[pipe_index++] = process_index - 1;
                        p->num_pipes++;
                        p->pipe_call = T;
                        entering_pipe = T;
                } else if (cmd[i] == ' ') {
                        if (entering_space == F && p->out_redir_call == F) {
                                process_char_index = 0;
                                entering_space = T;
                                process_index++;
                        }
                } else if (cmd[i] == '$') {
                        var_call = T;
                } else {
                        if (p->out_redir_call == T && var_call == F)
                                p->out_file[process_char_index++] = cmd[i];
                        else if (p->out_redir_call == F && var_call == F)
                                p->processes[process_index][process_char_index++] = cmd[i];
                        else if (var_call == T) {
                                char c = cmd[i];
                                int set_index = (int)c - ASCII_CONV;
                                p->processes[process_index] = set[set_index];
                                var_call = F;
                        }
                        entering_space = F;
                        entering_out = F;
                        entering_pipe = F;
                }
        }
        if (p->pipe_call == T) {
                init_multi_processes(p);
                split_processes(p, pipes);
        }
        return 0;
}

void cd(struct process *p, char *cmd)
{       
        int status;
        char *homedir = getenv("HOME");

        if (p->processes[FILE_DEST_INDEX] == NULL)
                chdir(homedir);
        else {
                if (chdir(p->processes[FILE_DEST_INDEX]) == -1) {
                        fprintf(stderr, "Error: cannot cd into directory\n");
                        status = 1;
                }
        }

        fprintf(stderr, "+ completed '%s' [%d]\n", cmd, status);
}

void reg_command(struct process *p, char *cmd)
{
        pid_t pid;

        pid = fork();
        if (pid == 0) {
                // Child process
                execvp(p->processes[START_INDEX], p->processes);
                fprintf(stderr, "Error: command not found\n");
                exit(1);
        } else if (pid > 0) {
                // Parent process
                int status;
                waitpid(pid, &status, 0);
                fprintf(stderr, "+ completed '%s' [%d]\n",
                        cmd, WEXITSTATUS(status));
        } else {
                perror("fork");
                exit(1);
        }
}

void init_process(struct process *p)
{
        for (int i = 0; i < ARG_MAX; i++) {
                p->processes[i] = malloc(TOKEN_MAX * sizeof(char));
                memset(p->processes[i], EMPTY_CHAR, TOKEN_MAX);
        }
        p->out_redir_call = F;
        p->out_file = malloc(TOKEN_MAX * sizeof(char));
        memset(p->out_file, EMPTY_CHAR, TOKEN_MAX);
        p->pipe_call = 0;
        p->num_pipes = 0;
}


void free_process(struct process *p)
{
        free(p->processes);
}

void out_redir(struct process *p, char *cmd)
{
        pid_t pid;

        pid = fork();
        if (pid == 0) {
                // Child process
                int fd;
                if (access(p->out_file, F_OK) == 0) {           // If out_file exists
                        fd = open(p->out_file,
                                  O_WRONLY | O_TRUNC,           // Truncate existing file
                                  DEFAULT_PERMISSIONS);
                } else {                                        // If out_file doesn't exist
                        fd = open(p->out_file,
                                  O_WRONLY | O_CREAT,           // Create file
                                  DEFAULT_PERMISSIONS);
                }
                dup2(fd, STDOUT_FILENO);
                close(fd);
                printf("%s\n", arg_instring(p));
                exit(0);
        } else if (pid > 0) {
                // Parent process
                int status;
                waitpid(pid, &status, 0);
                fprintf(stderr, "+ completed '%s' [%d]\n",
                        cmd, WEXITSTATUS(status));
        } else {
                perror("fork");
                exit(1);
        }
}

char *arg_instring(struct process *p)
{
        int i = 1;
        int c = 0;
        char *arg_string = malloc(CMDLINE_MAX * sizeof(char));

        while (p->processes[i] != NULL) {
                if (i > 1)
                        arg_string[c++] = ' ';
                for (int j = 0; j < TOKEN_MAX; j++) {
                        if (p->processes[i][j] == 0)
                                break;
                        else
                                arg_string[c++] = p->processes[i][j];
                }
                i++;
        }
        return arg_string;
}

void piping(struct process *p, char *cmd)
{
        int fd[p->num_pipes][2];
        pid_t pid[p->num_pipes + 1];    // fork for each process, processes = num pipes + 1

        for (int i = 0; i < p->num_pipes; i++) {
                if (pipe(fd[i]) < 0)
                        exit(2);
        }

        for (int i = 0; i < p->num_pipes + 1; i++) {
                pid[i] = fork();
        }

        if (pid[0] == 0) {
                for (int j = 0; j < p->num_pipes; j++) {
                        if (j == 0) {
                                close(fd[j][READ]);
                        } else {
                                close(fd[j][READ]);
                                close(fd[j][WRITE]);
                        }
                }
                dup2(fd[0][WRITE], STDOUT_FILENO);
                close(fd[0][WRITE]);
                execvp(p->multi_process[0][START_INDEX], p->multi_process[0]);
                exit(2);
        }
        for (int i = 1; i < p->num_pipes + 1; i++) {
                if (pid[i] == 0) {
                        if (i == p->num_pipes) 
                                dup2(fd[i-1][READ], STDIN_FILENO);
                        else 
                                dup2(fd[i][WRITE], STDOUT_FILENO);
                        for (int j = 0; j < p->num_pipes; j++) {
                                close(fd[j][READ]);
                                close(fd[j][WRITE]);
                        }
                        execvp(p->multi_process[i][START_INDEX], p->multi_process[i]);
                        exit(2);
                }
        }
        for (int i = 0; i < p->num_pipes; i++) {
                close(fd[i][READ]);
                close(fd[i][WRITE]);
        }
        for (int i = 0; i < p->num_pipes + 1; i++) {
                int statuses[MAX_PIPE_PROCESSES];
                if (pid[i] > 0) {
                        int status;
                        waitpid(pid[i], &status, 0);
                        statuses[i] = WEXITSTATUS(status);

                } else {
                        perror("fork");
                        exit(2);
                }
                if (i == p->num_pipes) {
                        fprintf(stderr, "+ completed '%s' [%d]",
                        cmd, statuses[0]);
                        for (int j = 1; j <= p->num_pipes; j++) {
                                fprintf(stderr, "[%d]", statuses[j]);
                        }
                        fprintf(stderr, "\n");
                }
        }
}

void split_processes(struct process *p, int array[MAX_PIPE_PROCESSES])
{
        int arg_index = 0;

        for (int i = 0; i < MAX_PIPE_PROCESSES; i++) {
                if (array[i] == 0)
                        break;
                int str_index = 0;
                for (arg_index = arg_index; arg_index <= array[i]; arg_index++) {
                        p->multi_process[i][str_index++] = p->processes[arg_index];
                }
                arg_index = array[i] + 1;
        }
}

void init_multi_processes(struct process *p)
{
        for (int i = 0; i < MAX_PIPE_PROCESSES; i++) {
                p->multi_process[i] = malloc(ARG_MAX * sizeof(char*));
        }
}

void set_var(struct process *p, char *set[MAX_VARIABLES])
{
        char c = p->processes[VAR_INDEX][START_INDEX];
        int i = (int)c - ASCII_CONV;

        set[i] = p->processes[VAR_INDEX + 1];
}