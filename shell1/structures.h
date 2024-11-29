#ifndef STRUCTURES_H
#define STRUCTURES_H

typedef enum {
    OP_NONE,
    OP_SEQUENCE,    // ;
    OP_AND,         // &&
    OP_OR,          // ||
    OP_PIPE,        // |
    OP_BACKGROUND   // &
} OperatorType;

typedef struct Command {
    char **argv;                // Commaand line arguments
    char *input_file;           // File for redirecting input
    char *output_file;          // File for redirecting output
    char *append_file;          // Файл для добавления вывода
    int is_background;          // Flag for background process
    struct Command *next;       // Next Command for Conveyer
    int is_subshell;            // Flag for subshell
    struct Job *subjob;         // Вложенный Job для подшелла
} Command;

typedef struct Job {
    Command *first_command;     // First Command or Conveyer
    OperatorType operator;      // Operator between the tasks
    struct Job *next;           // Next Job
} Job;

#endif
