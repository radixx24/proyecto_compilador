#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "ast.h"

typedef struct {
    Lexer lexer;
    Token current;
    int has_error;
    int control_depth;
    int max_control_depth;
} Parser;

void parser_init(Parser *parser, const char *source);
ASTNode *parse_program(Parser *parser);

#endif