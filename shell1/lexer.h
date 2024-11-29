#ifndef LEXER_H
#define LEXER_H

typedef enum {
    TOKEN_WORD,
    TOKEN_OPERATOR,
    TOKEN_EOF,
    TOKEN_ERROR
} TokenType;

typedef struct {
    TokenType type;
    char *text;
} Token;

typedef struct {
    char *input_stream;
    int input_pos;
} Lexer;

void init_lexer(Lexer *lexer, char *input);
Token *get_next_token(Lexer *lexer);
void free_token(Token *token);

#endif
