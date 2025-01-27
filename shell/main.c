#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include "parser.h"
#include "executor.h"
#include "colors.h"

void sigint_handler(int signo) // Ctrl-C
{
    printf("\n");
}

void sigchld_handler(int signo) // Zombi prevention
{
    int status;
    while (waitpid(-1, &status, WNOHANG) > 0)
        ;
}

int main()
{
    char *line = NULL;
    size_t len = 0;

    signal(SIGINT, sigint_handler);
    signal(SIGCHLD, sigchld_handler);

    print_rainbow_text("Hello, dear user! Happy to see you on MyShell.");
    printf("Typos and logic errors in cmd's are highlited in attention messages.\n");
    print_rainbow_text("Enjoy.\n");

    while (1)
    {
        printf("shell> ");
        fflush(stdout);
        ssize_t nread = getline(&line, &len, stdin);
        if (nread == -1)
        {
            break;
        }

        if (nread > 0 && line[nread - 1] == '\n')
        {
            line[nread - 1] = '\0';
        }

        Job *job = parse_line(line);
        if (job == NULL)
        {
            continue;
        }

        execute_job(job);
        free_job(job);
    }

    free(line);
    return 0;
}