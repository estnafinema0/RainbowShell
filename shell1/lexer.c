#include "lexer.h"
#include "colors.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void init_lexer(Lexer *lexer, char *input) {
    lexer->input_stream = input;
    lexer->input_pos = 0;
}

Token *get_next_token(Lexer *lexer) {
    Token *token = malloc(sizeof(Token));
    token->text = NULL;

    while (lexer->input_stream[lexer->input_pos] && isspace(lexer->input_stream[lexer->input_pos])) {
        lexer->input_pos++; // skip spaces
    }

    if (lexer->input_stream[lexer->input_pos] == '\0') {
        token->type = TOKEN_EOF; // make EOF
        return token;
    }

    char c = lexer->input_stream[lexer->input_pos];

    if (strchr("|&;()<>", c)) { //operators
        int start = lexer->input_pos;
        lexer->input_pos++;

        if ((c == '|' && lexer->input_stream[lexer->input_pos] == '|') ||
            (c == '&' && lexer->input_stream[lexer->input_pos] == '&') ||
            (c == '>' && lexer->input_stream[lexer->input_pos] == '>')) {
            lexer->input_pos++; //process >> && ||
        }

        int len = lexer->input_pos - start;
        token->type = TOKEN_OPERATOR;
        token->text = strndup(&lexer->input_stream[start], len);
        return token;
    }

    if (c == '"' || c == '\'') {
        char quote_char = c;
        lexer->input_pos++;
        int start = lexer->input_pos;
        while (lexer->input_stream[lexer->input_pos] && lexer->input_stream[lexer->input_pos] != quote_char) {
            lexer->input_pos++;
        }
        if (lexer->input_stream[lexer->input_pos] == quote_char) {
            int len = lexer->input_pos - start;
            token->type = TOKEN_WORD;
            token->text = strndup(&lexer->input_stream[start], len);
            lexer->input_pos++; // Skip the closing quotation mark
            return token;
        } else {
            print_error_message("Lexer: ", "no closed quotation mark\n");
            token->type = TOKEN_ERROR;
            return token;
        }
    }

    if (isprint(c)) { // cmd or argument
        int start = lexer->input_pos;
        while (lexer->input_stream[lexer->input_pos] && !isspace(lexer->input_stream[lexer->input_pos]) &&
               !strchr("|&;()<>", lexer->input_stream[lexer->input_pos])) {
            if (lexer->input_stream[lexer->input_pos] == '"' || lexer->input_stream[lexer->input_pos] == '\'') {
                // Processing "" inside
                char quote_char = lexer->input_stream[lexer->input_pos];
                lexer->input_pos++;
                while (lexer->input_stream[lexer->input_pos] && lexer->input_stream[lexer->input_pos] != quote_char) {
                    lexer->input_pos++;
                }
                if (lexer->input_stream[lexer->input_pos] == quote_char) {
                    lexer->input_pos++;
                } else {
                    print_error_message("Lexer: ", "no closed quotation mark\n");
                    token->type = TOKEN_ERROR;
                    return token;
                }
            } else {
                lexer->input_pos++;
            }
        }
        int len = lexer->input_pos - start;
        token->type = TOKEN_WORD;
        token->text = strndup(&lexer->input_stream[start], len);
        return token;
    }

    print_error_message("Lexer: ", "error\n"); //symbol c
    token->type = TOKEN_ERROR;
    return token;
}

void free_token(Token *token) {
    if (token->text) {
        free(token->text);
    }
    free(token);
}
