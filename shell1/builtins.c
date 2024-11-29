#include "builtins.h"
#include "colors.h"
#include <string.h>
#include <unistd.h>
#include <stdio.h>

int is_builtin(char *cmd) {
    return strcmp(cmd, "cd") == 0;
}

int execute_builtin(Command *cmd) {
    if (strcmp(cmd->argv[0], "cd") == 0) {
        if (cmd->argv[1] == NULL) {
            print_error_message("Attention:", " cd: expected an argument\n");
            return 1;
        } else {
            if (chdir(cmd->argv[1]) != 0) {
                perror("Error: cd");
                return 1;
            }
        }
    }
    return 0;
}
