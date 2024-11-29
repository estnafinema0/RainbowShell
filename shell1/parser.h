#ifndef PARSER_H
#define PARSER_H

#include "structures.h"

Job *parse_line(char *line);
void free_command(Command *cmd);
void free_job(Job *job) ;

#endif
