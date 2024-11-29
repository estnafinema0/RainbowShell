#include "parser.h"
#include "lexer.h"
#include "colors.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static Token *current_token;
static Lexer lexer_instance;

static int has_lexer_error = 0;

static void get_next() {
    if (current_token) {
        free_token(current_token);
    }
    current_token = get_next_token(&lexer_instance);
    if (current_token->type == TOKEN_ERROR) {
        has_lexer_error = 1;
    }
}

static int match_operator(const char *op) {
    if (current_token->type == TOKEN_OPERATOR &&
        strcmp(current_token->text, op) == 0) {
        get_next();
        return 1;
    }
    return 0;
}

Job *parse_line(char *line);
static Job *parse_job();
void free_command(Command *cmd);
void free_job(Job *job) ;
static Command *parse_pipeline();
static Command *parse_simple_command();
static Command *parse_command_with_redirections();
static void parse_redirections(Command *cmd);

Job *parse_line(char *line) {
    init_lexer(&lexer_instance, line);
    current_token = NULL;
    has_lexer_error = 0;
    get_next();
    if (current_token->type == TOKEN_EOF) {
        return NULL;
    }
    if (has_lexer_error) {
        print_error_message("Lexer: ", "incorrect input\n");
        free_token(current_token);
        return NULL;
    }
    Job *job = parse_job();
    if (job == NULL || has_lexer_error) {  // when error occured in lexer
        free_token(current_token);
        return NULL;
    }
    if (current_token->type != TOKEN_EOF) {
        fprintf(stderr, "Parcer: Syntax error\n" ); // current_token->text error
        free_job(job);
        job = NULL;
    }
    free_token(current_token);
    return job;
}

static Job *parse_job() {
    if (has_lexer_error) return NULL;

    Job *job = malloc(sizeof(Job));
    memset(job, 0, sizeof(Job));
    job->first_command = parse_command_with_redirections();

    if (job->first_command == NULL) {
        free(job);
        return NULL;
    }

    if (match_operator("&&")) {
        job->operator = OP_AND;
    } else if (match_operator("||")) {
        job->operator = OP_OR;
    } else if (match_operator(";")) {
        job->operator = OP_SEQUENCE;
    } else if (match_operator("&")) {
        job->operator = OP_BACKGROUND;
        job->first_command->is_background = 1;
    }

    if (job->operator != OP_NONE) {
        job->next = parse_job();
        if (job->next == NULL) {
            free_job(job);
            return NULL;
        }
    }

    return job;
}

static Command *parse_command_with_redirections() {
    if (has_lexer_error) return NULL;

    Command *cmd = NULL;
    if (match_operator("(")) {
        Job *subjob = parse_job();
        if (subjob == NULL) { // Subshell error
            return NULL;
        }
        if (!match_operator(")")) {
            fprintf(stderr, "Ожидалась ')'\n");
            free_job(subjob);
            return NULL;
        }
        // cmd for doing subshell
        cmd = malloc(sizeof(Command)); 
        memset(cmd, 0, sizeof(Command));
        cmd->argv = malloc(2 * sizeof(char *));
        cmd->argv[0] = strdup("subshell");
        cmd->argv[1] = NULL;
        cmd->next = NULL;
        cmd->is_subshell = 1;
        cmd->subjob = subjob;
    } else {
        cmd = parse_pipeline();
        if (cmd == NULL) { //Conveyer error
            return NULL;
        }
    }

    parse_redirections(cmd);
    return cmd;
}

static Command *parse_pipeline() {
    if (has_lexer_error) return NULL;

    Command *cmd = parse_simple_command();

    if (cmd == NULL) { // simple cmd error
        return NULL;
    }

    if (match_operator("|")) {
        Command *next_cmd = parse_pipeline();
        if (next_cmd == NULL) { // next cmd error
            free_command(cmd);
            return NULL;
        }
        cmd->next = next_cmd;
    }

    return cmd;
}

static Command *parse_simple_command() {
    if (has_lexer_error) return NULL;

    if (current_token->type == TOKEN_ERROR) {
        print_error_message("Lexer: ", "incorrect token\n");
        has_lexer_error = 1;
        return NULL;
    }

    if (current_token->type != TOKEN_WORD) {
        print_error_message("Parser:", " command expected\n");// current_token->text instead of expected command
        return NULL;
    }

    Command *cmd = malloc(sizeof(Command));
    memset(cmd, 0, sizeof(Command));

    int argc = 0;
    int capacity = 10;
    cmd->argv = malloc(capacity * sizeof(char *));

    while (current_token->type == TOKEN_WORD) {
        if (argc >= capacity - 1) {
            capacity *= 2;
            cmd->argv = realloc(cmd->argv, capacity * sizeof(char *));
        }
        cmd->argv[argc++] = strdup(current_token->text);
        get_next();
        if (has_lexer_error) {
            print_error_message("Lexer: ", "incorrect token\n");
            free_command(cmd);
            return NULL;
        }
    }
    cmd->argv[argc] = NULL;

    return cmd;
}

static void parse_redirections(Command *cmd) {
    if (cmd == NULL || has_lexer_error) {
        return;
    }

    while (current_token->type == TOKEN_OPERATOR &&
           (strcmp(current_token->text, "<") == 0 ||
            strcmp(current_token->text, ">") == 0 ||
            strcmp(current_token->text, ">>") == 0)) {

        char *op = strdup(current_token->text);
        get_next();
        if (has_lexer_error) {
            print_error_message("Lexer: ", "incorrect token\n");
            free(op);
            return;
        }

        if (current_token->type != TOKEN_WORD) {
            fprintf(stderr, "COLOR_BOLD_REDParser: COLOR_RESETfile name expected after '%s'\n", op);
            free(op);
            return;
        }

        char *filename = strdup(current_token->text);
        get_next();
        if (has_lexer_error) {
            print_error_message("Lexer: ", "incorrect token\n\n");
            free(op);
            free(filename);
            return;
        }

        if (strcmp(op, "<") == 0) {
            cmd->input_file = filename;
        } else if (strcmp(op, ">") == 0) {
            cmd->output_file = filename;
        } else if (strcmp(op, ">>") == 0) {
            cmd->append_file = filename;
        }

        free(op);
    }
}

void free_command(Command *cmd) {
    if (cmd == NULL) return;

    if (cmd->argv) {
        for (int i = 0; cmd->argv[i]; i++) {
            free(cmd->argv[i]);
        }
        free(cmd->argv);
    }
    if (cmd->input_file) {
        free(cmd->input_file);
    }
    if (cmd->output_file) {
        free(cmd->output_file);
    }
    if (cmd->append_file) {
        free(cmd->append_file);
    }
    if (cmd->is_subshell && cmd->subjob) {
        free_job(cmd->subjob);
    }
    if (cmd->next) {
        free_command(cmd->next);
    }
    free(cmd);
}

void free_job(Job *job) {
    if (job == NULL) return;

    if (job->first_command) {
        free_command(job->first_command);
    }
    if (job->next) {
        free_job(job->next);
    }
    free(job);
}
