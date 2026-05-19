#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "lexer.h"

static char lexer_peek(Lexer *lexer) {
    return lexer->source[lexer->index];
}

static char lexer_peek_next(Lexer *lexer) {
    if (lexer->source[lexer->index] == '\0') {
        return '\0';
    }

    return lexer->source[lexer->index + 1];
}

static char lexer_advance(Lexer *lexer) {
    char current;

    current = lexer->source[lexer->index];

    if (current == '\0') {
        return current;
    }

    lexer->index = lexer->index + 1;

    if (current == '\n') {
        lexer->line = lexer->line + 1;
        lexer->column = 1;
    } else {
        lexer->column = lexer->column + 1;
    }

    return current;
}

static Token make_token(TokenType type, const char *lexeme, int line, int column) {
    Token token;

    token.type = type;

    strncpy(token.lexeme, lexeme, MAX_LEXEME - 1);
    token.lexeme[MAX_LEXEME - 1] = '\0';

    token.line = line;
    token.column = column;

    return token;
}

static Token keyword_or_id(const char *lexeme, int line, int column) {
    if (strcmp(lexeme, "program") == 0) {
        return make_token(TOKEN_PROGRAM, lexeme, line, column);
    }

    if (strcmp(lexeme, "int") == 0) {
        return make_token(TOKEN_INT, lexeme, line, column);
    }

    if (strcmp(lexeme, "float") == 0) {
        return make_token(TOKEN_FLOAT, lexeme, line, column);
    }

    if (strcmp(lexeme, "bool") == 0) {
        return make_token(TOKEN_BOOL, lexeme, line, column);
    }

    if (strcmp(lexeme, "string") == 0) {
        return make_token(TOKEN_STRING, lexeme, line, column);
    }

    if (strcmp(lexeme, "if") == 0) {
        return make_token(TOKEN_IF, lexeme, line, column);
    }

    if (strcmp(lexeme, "else") == 0) {
        return make_token(TOKEN_ELSE, lexeme, line, column);
    }

    if (strcmp(lexeme, "for") == 0) {
        return make_token(TOKEN_FOR, lexeme, line, column);
    }

    if (strcmp(lexeme, "while") == 0) {
        return make_token(TOKEN_WHILE, lexeme, line, column);
    }

    if (strcmp(lexeme, "do") == 0) {
        return make_token(TOKEN_DO, lexeme, line, column);
    }

    if (strcmp(lexeme, "true") == 0 || strcmp(lexeme, "false") == 0) {
        return make_token(TOKEN_BOOL_LITERAL, lexeme, line, column);
    }

    return make_token(TOKEN_ID, lexeme, line, column);
}

static void skip_whitespace_and_comments(Lexer *lexer) {
    int skipping;

    skipping = 1;

    while (skipping == 1) {
        skipping = 0;

        while (isspace((unsigned char)lexer_peek(lexer))) {
            lexer_advance(lexer);
            skipping = 1;
        }

        if (lexer_peek(lexer) == '/' && lexer_peek_next(lexer) == '/') {
            while (lexer_peek(lexer) != '\n' && lexer_peek(lexer) != '\0') {
                lexer_advance(lexer);
            }

            skipping = 1;
        }

        if (lexer_peek(lexer) == '/' && lexer_peek_next(lexer) == '*') {
            lexer_advance(lexer);
            lexer_advance(lexer);

            while (lexer_peek(lexer) != '\0') {
                if (lexer_peek(lexer) == '*' && lexer_peek_next(lexer) == '/') {
                    lexer_advance(lexer);
                    lexer_advance(lexer);
                    break;
                }

                lexer_advance(lexer);
            }

            skipping = 1;
        }
    }
}

void lexer_init(Lexer *lexer, const char *source) {
    lexer->source = source;
    lexer->index = 0;
    lexer->line = 1;
    lexer->column = 1;
}

Token lexer_next_token(Lexer *lexer) {
    char c;
    int line;
    int column;
    char buffer[MAX_LEXEME];
    int length;
    int has_dot;

    skip_whitespace_and_comments(lexer);

    line = lexer->line;
    column = lexer->column;
    c = lexer_peek(lexer);

    if (c == '\0') {
        return make_token(TOKEN_EOF, "", line, column);
    }

    if (isalpha((unsigned char)c) || c == '_') {
        length = 0;

        while (isalnum((unsigned char)lexer_peek(lexer)) || lexer_peek(lexer) == '_') {
            if (length < MAX_LEXEME - 1) {
                buffer[length] = lexer_peek(lexer);
                length = length + 1;
            }

            lexer_advance(lexer);
        }

        buffer[length] = '\0';

        return keyword_or_id(buffer, line, column);
    }

    if (isdigit((unsigned char)c)) {
        length = 0;
        has_dot = 0;

        while (isdigit((unsigned char)lexer_peek(lexer)) || lexer_peek(lexer) == '.') {
            if (lexer_peek(lexer) == '.') {
                if (has_dot == 1) {
                    break;
                }

                has_dot = 1;
            }

            if (length < MAX_LEXEME - 1) {
                buffer[length] = lexer_peek(lexer);
                length = length + 1;
            }

            lexer_advance(lexer);
        }

        buffer[length] = '\0';

        if (has_dot == 1) {
            return make_token(TOKEN_FLOAT_LITERAL, buffer, line, column);
        }

        return make_token(TOKEN_INT_LITERAL, buffer, line, column);
    }

    if (c == '"') {
        lexer_advance(lexer);
        length = 0;

        while (lexer_peek(lexer) != '"' && lexer_peek(lexer) != '\0' && lexer_peek(lexer) != '\n') {
            if (length < MAX_LEXEME - 1) {
                buffer[length] = lexer_peek(lexer);
                length = length + 1;
            }

            lexer_advance(lexer);
        }

        buffer[length] = '\0';

        if (lexer_peek(lexer) != '"') {
            return make_token(TOKEN_ERROR, "cadena sin cerrar", line, column);
        }

        lexer_advance(lexer);

        return make_token(TOKEN_STRING_LITERAL, buffer, line, column);
    }

    lexer_advance(lexer);

    if (c == '+') {
        return make_token(TOKEN_PLUS, "+", line, column);
    }

    if (c == '-') {
        return make_token(TOKEN_MINUS, "-", line, column);
    }

    if (c == '*') {
        return make_token(TOKEN_STAR, "*", line, column);
    }

    if (c == '/') {
        return make_token(TOKEN_SLASH, "/", line, column);
    }

    if (c == ';') {
        return make_token(TOKEN_SEMICOLON, ";", line, column);
    }

    if (c == ',') {
        return make_token(TOKEN_COMMA, ",", line, column);
    }

    if (c == '(') {
        return make_token(TOKEN_LPAREN, "(", line, column);
    }

    if (c == ')') {
        return make_token(TOKEN_RPAREN, ")", line, column);
    }

    if (c == '{') {
        return make_token(TOKEN_LBRACE, "{", line, column);
    }

    if (c == '}') {
        return make_token(TOKEN_RBRACE, "}", line, column);
    }

    if (c == '=') {
        if (lexer_peek(lexer) == '=') {
            lexer_advance(lexer);
            return make_token(TOKEN_EQUAL, "==", line, column);
        }

        return make_token(TOKEN_ASSIGN, "=", line, column);
    }

    if (c == '!') {
        if (lexer_peek(lexer) == '=') {
            lexer_advance(lexer);
            return make_token(TOKEN_NOT_EQUAL, "!=", line, column);
        }

        return make_token(TOKEN_NOT, "!", line, column);
    }

    if (c == '<') {
        if (lexer_peek(lexer) == '=') {
            lexer_advance(lexer);
            return make_token(TOKEN_LESS_EQUAL, "<=", line, column);
        }

        return make_token(TOKEN_LESS, "<", line, column);
    }

    if (c == '>') {
        if (lexer_peek(lexer) == '=') {
            lexer_advance(lexer);
            return make_token(TOKEN_GREATER_EQUAL, ">=", line, column);
        }

        return make_token(TOKEN_GREATER, ">", line, column);
    }

    if (c == '&') {
        if (lexer_peek(lexer) == '&') {
            lexer_advance(lexer);
            return make_token(TOKEN_AND, "&&", line, column);
        }

        return make_token(TOKEN_ERROR, "se esperaba &&", line, column);
    }

    if (c == '|') {
        if (lexer_peek(lexer) == '|') {
            lexer_advance(lexer);
            return make_token(TOKEN_OR, "||", line, column);
        }

        return make_token(TOKEN_ERROR, "se esperaba ||", line, column);
    }

    buffer[0] = c;
    buffer[1] = '\0';

    return make_token(TOKEN_ERROR, buffer, line, column);
}

const char *token_type_to_string(TokenType type) {
    if (type == TOKEN_EOF) {
        return "TOKEN_EOF";
    }

    if (type == TOKEN_ID) {
        return "TOKEN_ID";
    }

    if (type == TOKEN_INT_LITERAL) {
        return "TOKEN_INT_LITERAL";
    }

    if (type == TOKEN_FLOAT_LITERAL) {
        return "TOKEN_FLOAT_LITERAL";
    }

    if (type == TOKEN_STRING_LITERAL) {
        return "TOKEN_STRING_LITERAL";
    }

    if (type == TOKEN_BOOL_LITERAL) {
        return "TOKEN_BOOL_LITERAL";
    }

    if (type == TOKEN_PROGRAM) {
        return "TOKEN_PROGRAM";
    }

    if (type == TOKEN_INT) {
        return "TOKEN_INT";
    }

    if (type == TOKEN_FLOAT) {
        return "TOKEN_FLOAT";
    }

    if (type == TOKEN_BOOL) {
        return "TOKEN_BOOL";
    }

    if (type == TOKEN_STRING) {
        return "TOKEN_STRING";
    }

    if (type == TOKEN_IF) {
        return "TOKEN_IF";
    }

    if (type == TOKEN_ELSE) {
        return "TOKEN_ELSE";
    }

    if (type == TOKEN_FOR) {
        return "TOKEN_FOR";
    }

    if (type == TOKEN_WHILE) {
        return "TOKEN_WHILE";
    }

    if (type == TOKEN_DO) {
        return "TOKEN_DO";
    }

    if (type == TOKEN_PLUS) {
        return "TOKEN_PLUS";
    }

    if (type == TOKEN_MINUS) {
        return "TOKEN_MINUS";
    }

    if (type == TOKEN_STAR) {
        return "TOKEN_STAR";
    }

    if (type == TOKEN_SLASH) {
        return "TOKEN_SLASH";
    }

    if (type == TOKEN_ASSIGN) {
        return "TOKEN_ASSIGN";
    }

    if (type == TOKEN_EQUAL) {
        return "TOKEN_EQUAL";
    }

    if (type == TOKEN_NOT_EQUAL) {
        return "TOKEN_NOT_EQUAL";
    }

    if (type == TOKEN_LESS) {
        return "TOKEN_LESS";
    }

    if (type == TOKEN_LESS_EQUAL) {
        return "TOKEN_LESS_EQUAL";
    }

    if (type == TOKEN_GREATER) {
        return "TOKEN_GREATER";
    }

    if (type == TOKEN_GREATER_EQUAL) {
        return "TOKEN_GREATER_EQUAL";
    }

    if (type == TOKEN_AND) {
        return "TOKEN_AND";
    }

    if (type == TOKEN_OR) {
        return "TOKEN_OR";
    }

    if (type == TOKEN_NOT) {
        return "TOKEN_NOT";
    }

    if (type == TOKEN_SEMICOLON) {
        return "TOKEN_SEMICOLON";
    }

    if (type == TOKEN_COMMA) {
        return "TOKEN_COMMA";
    }

    if (type == TOKEN_LPAREN) {
        return "TOKEN_LPAREN";
    }

    if (type == TOKEN_RPAREN) {
        return "TOKEN_RPAREN";
    }

    if (type == TOKEN_LBRACE) {
        return "TOKEN_LBRACE";
    }

    if (type == TOKEN_RBRACE) {
        return "TOKEN_RBRACE";
    }

    return "TOKEN_ERROR";
}

void print_all_tokens(const char *source) {
    Lexer lexer;
    Token token;
    int finished;

    lexer_init(&lexer, source);

    finished = 0;

    while (finished == 0) {
        token = lexer_next_token(&lexer);

        printf(
            "%-24s lexema='%-15s' linea=%d columna=%d\n",
            token_type_to_string(token.type),
            token.lexeme,
            token.line,
            token.column
        );

        if (token.type == TOKEN_EOF || token.type == TOKEN_ERROR) {
            finished = 1;
        }
    }
}