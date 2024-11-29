#ifndef EXECUTOR_H
#define EXECUTOR_H

#include "structures.h"

int execute_job(Job *job);
int execute_command(Command *cmd);
int execute_pipeline(Command *cmd);
void restore_redirections(int saved_stdin, int saved_stdout);
void setup_redirections_for_pipes(Command *cmd);
void setup_redirections(Command *cmd, int *saved_stdin, int *saved_stdout);

#endif 
