#include "executor.h"
#include "structures.h"
#include "builtins.h"
#include "colors.h"
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int execute_job(Job *job);
int execute_command(Command *cmd);
int execute_pipeline(Command *cmd);
void restore_redirections(int saved_stdin, int saved_stdout);
void setup_redirections_for_pipes(Command *cmd);
void setup_redirections(Command *cmd, int *saved_stdin, int *saved_stdout);

void setup_redirections_for_pipes(Command *cmd) {
    if (cmd->input_file) {
        int fd = open(cmd->input_file, O_RDONLY);
        if (fd < 0) {
            perror("open input file");
            exit(EXIT_FAILURE);
        }
        dup2(fd, STDIN_FILENO);
        close(fd);
    }
    if (cmd->output_file) {
        int fd = open(cmd->output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd < 0) {
            perror("open output file");
            exit(EXIT_FAILURE);
        }
        dup2(fd, STDOUT_FILENO);
        close(fd);
    }
    if (cmd->append_file) {
        int fd = open(cmd->append_file, O_WRONLY | O_CREAT | O_APPEND, 0644);
        if (fd < 0) {
            perror("Error: open append file");
            exit(EXIT_FAILURE);
        }
        dup2(fd, STDOUT_FILENO);
        close(fd);
    }
}
int execute_job(Job *job) {
    int status = 0;
    Job *current_job = job;

    while (current_job != NULL) {
        if (current_job->first_command == NULL) {
            print_error_message("Executor: ",  " no cmd to execute\n");
            return -1;
        }
        if (current_job->first_command->is_subshell) {
            
            pid_t pid = fork(); // for subshell
            if (pid == 0) {
                exit(execute_job(current_job->first_command->subjob));
            } else if (pid < 0) {
                perror("Error: fork");
                status = -1;
            } else {
                waitpid(pid, &status, 0);
                status = WEXITSTATUS(status);
            }
        } else if (current_job->first_command->next != NULL) { //Conveyer
            status = execute_pipeline(current_job->first_command);
        } else { //Single cmd
            status = execute_command(current_job->first_command);
        }

        if (current_job->operator == OP_AND) {
            if (status != 0) { //!!
                break;
            }
        } else if (current_job->operator == OP_OR) {
            if (status == 0) { //!!
                break;
            }
        }

        current_job = current_job->next;
    }

    return status;
}


void setup_redirections(Command *cmd, int *saved_stdin, int *saved_stdout) {
    if (cmd->input_file) {
        *saved_stdin = dup(STDIN_FILENO);
        int fd = open(cmd->input_file, O_RDONLY);
        if (fd < 0) {
            perror("Error: open input file");
            exit(EXIT_FAILURE);
        }
        dup2(fd, STDIN_FILENO);
        close(fd);
    }
    if (cmd->output_file || cmd->append_file) {
        *saved_stdout = dup(STDOUT_FILENO);
        int fd;
        if (cmd->output_file) {
            fd = open(cmd->output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        } else {
            fd = open(cmd->append_file, O_WRONLY | O_CREAT | O_APPEND, 0644);
        }
        if (fd < 0) {
            perror("Error: open output file");
            exit(EXIT_FAILURE);
        }
        dup2(fd, STDOUT_FILENO);
        close(fd);
    }
}

void restore_redirections(int saved_stdin, int saved_stdout) {
    if (saved_stdin != -1) {
        dup2(saved_stdin, STDIN_FILENO);
        close(saved_stdin);
    }
    if (saved_stdout != -1) {
        dup2(saved_stdout, STDOUT_FILENO);
        close(saved_stdout);
    }
}

int execute_command(Command *cmd) {
    int status = 0;
    int saved_stdin = -1, saved_stdout = -1;

    if (cmd->is_background) {
        pid_t pid = fork();
        if (pid == 0) {//Background
            setpgid(0, 0);
            int devnull = open("/dev/null", O_RDONLY);
            dup2(devnull, STDIN_FILENO);
            close(devnull);
            signal(SIGINT, SIG_IGN);
            setup_redirections(cmd, &saved_stdin, &saved_stdout);
            if (is_builtin(cmd->argv[0])) {
                execute_builtin(cmd);
                exit(EXIT_SUCCESS);
            } else {
                execvp(cmd->argv[0], cmd->argv);
                perror("execvp");
                exit(EXIT_FAILURE);
            }
        } else if (pid < 0) {
            perror("fork");
            return -1;
        } else {
            printf(COLOR_MAGENTA"[Background process with с PID %d]"COLOR_RESET"\n" , pid);
            return 0;
        }
    } else { // Return to regular execution
        if (is_builtin(cmd->argv[0])) {
            setup_redirections(cmd, &saved_stdin, &saved_stdout);

            status = execute_builtin(cmd); // Save in parent process

            restore_redirections(saved_stdin, saved_stdout);
        } else {
            pid_t pid = fork();
            if (pid == 0) {
                setup_redirections(cmd, &saved_stdin, &saved_stdout);
                execvp(cmd->argv[0], cmd->argv);
                perror("execvp");
                exit(EXIT_FAILURE);
            } else if (pid < 0) {
                perror("fork");
                return -1;
            } else {
                waitpid(pid, &status, 0);
                status = WEXITSTATUS(status);
            }
        }
        return status;
    }
}

int execute_pipeline(Command *cmd) {
    int pipe_fd[2];
    pid_t pid;
    int in_fd = STDIN_FILENO;
    int status = 0;
    pid_t last_pid = 0;

    Command *current_cmd = cmd;
    while (current_cmd != NULL) {
        if (current_cmd->next != NULL) {
            if (pipe(pipe_fd) == -1) { //For next cmd
                perror("Error: pipe");
                return -1;
            }
        } else { //last cmd
            pipe_fd[0] = -1;
            pipe_fd[1] = -1;
        }

        pid = fork();
        if (pid == 0) {
            if (in_fd != STDIN_FILENO) {
                dup2(in_fd, STDIN_FILENO);
                close(in_fd);
            }
            if (current_cmd->next != NULL) {
                dup2(pipe_fd[1], STDOUT_FILENO);
                close(pipe_fd[1]);
                close(pipe_fd[0]);
            }
            int saved_stdin = -1, saved_stdout = -1;
            setup_redirections(current_cmd, &saved_stdin, &saved_stdout);

            if (is_builtin(current_cmd->argv[0])) {
                execute_builtin(current_cmd);
                exit(EXIT_SUCCESS);
            } else {
                execvp(current_cmd->argv[0], current_cmd->argv);
                perror("execvp");
                exit(EXIT_FAILURE);
            }
        } else if (pid < 0) {
            perror("fork");
            return -1;
        } else {
            if (in_fd != STDIN_FILENO) {
                close(in_fd);
            }
            if (current_cmd->next != NULL) {
                close(pipe_fd[1]);
                in_fd = pipe_fd[0];
            }
            last_pid = pid;
            current_cmd = current_cmd->next;
        }
    }

    // Waiting for all conveyers to end
    int wstatus;
    while ((pid = wait(&wstatus)) > 0) {
        if (pid == last_pid) {
            status = WIFEXITED(wstatus) ? WEXITSTATUS(wstatus) : -1;
        }
    }

    return status;
}
