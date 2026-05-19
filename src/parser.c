#include <stdio.h>
#include <string.h>

#include "parser.h"
#include "utils.h"

static void parser_advance(Parser *parser) {
    parser->current = lexer_next_token(&parser->lexer);

    if (parser->current.type == TOKEN_ERROR) {
        print_error("Lexico", parser->current.line, parser->current.column, parser->current.lexeme);
        parser->has_error = 1;
    }
}

static int parser_match(Parser *parser, TokenType type) {
    if (parser->current.type == type) {
        parser_advance(parser);
        return 1;
    }

    return 0;
}

static void parser_expect(Parser *parser, TokenType type, const char *message) {
    if (parser->current.type == type) {
        parser_advance(parser);
        return;
    }

    print_error("Sintactico", parser->current.line, parser->current.column, message);
    parser->has_error = 1;
}

static int token_is_type(TokenType type) {
    if (type == TOKEN_INT || type == TOKEN_FLOAT || type == TOKEN_BOOL || type == TOKEN_STRING) {
        return 1;
    }

    return 0;
}

static DataType parse_type(Parser *parser) {
    if (parser_match(parser, TOKEN_INT)) {
        return TYPE_INT;
    }

    if (parser_match(parser, TOKEN_FLOAT)) {
        return TYPE_FLOAT;
    }

    if (parser_match(parser, TOKEN_BOOL)) {
        return TYPE_BOOL;
    }

    if (parser_match(parser, TOKEN_STRING)) {
        return TYPE_STRING;
    }

    print_error("Sintactico", parser->current.line, parser->current.column, "se esperaba un tipo de dato");
    parser->has_error = 1;

    return TYPE_ERROR;
}

static void enter_control(Parser *parser) {
    parser->control_depth = parser->control_depth + 1;

    if (parser->control_depth > parser->max_control_depth) {
        parser->max_control_depth = parser->control_depth;
    }

    if (parser->control_depth > 3) {
        print_error("Sintactico", parser->current.line, parser->current.column, "el anidamiento excede 3 niveles");
        parser->has_error = 1;
    }
}

static void exit_control(Parser *parser) {
    if (parser->control_depth > 0) {
        parser->control_depth = parser->control_depth - 1;
    }
}

static ASTNode *parse_expression(Parser *parser);
static ASTNode *parse_statement(Parser *parser);
static ASTNode *parse_block(Parser *parser);

void parser_init(Parser *parser, const char *source) {
    lexer_init(&parser->lexer, source);

    parser->current = lexer_next_token(&parser->lexer);
    parser->has_error = 0;
    parser->control_depth = 0;
    parser->max_control_depth = 0;

    if (parser->current.type == TOKEN_ERROR) {
        print_error("Lexico", parser->current.line, parser->current.column, parser->current.lexeme);
        parser->has_error = 1;
    }
}

static ASTNode *parse_primary(Parser *parser) {
    ASTNode *node;
    Token token;

    token = parser->current;

    if (parser_match(parser, TOKEN_INT_LITERAL)) {
        node = create_node(NODE_LITERAL, token.lexeme, token.line);
        node->data_type = TYPE_INT;
        return node;
    }

    if (parser_match(parser, TOKEN_FLOAT_LITERAL)) {
        node = create_node(NODE_LITERAL, token.lexeme, token.line);
        node->data_type = TYPE_FLOAT;
        return node;
    }

    if (parser_match(parser, TOKEN_STRING_LITERAL)) {
        node = create_node(NODE_LITERAL, token.lexeme, token.line);
        node->data_type = TYPE_STRING;
        return node;
    }

    if (parser_match(parser, TOKEN_BOOL_LITERAL)) {
        node = create_node(NODE_LITERAL, token.lexeme, token.line);
        node->data_type = TYPE_BOOL;
        return node;
    }

    if (parser_match(parser, TOKEN_ID)) {
        return create_node(NODE_IDENTIFIER, token.lexeme, token.line);
    }

    if (parser_match(parser, TOKEN_LPAREN)) {
        node = parse_expression(parser);
        parser_expect(parser, TOKEN_RPAREN, "se esperaba ')' al cerrar expresion");
        return node;
    }

    print_error("Sintactico", parser->current.line, parser->current.column, "expresion primaria invalida");
    parser->has_error = 1;

    parser_advance(parser);

    return create_node(NODE_EMPTY, "error", token.line);
}

static ASTNode *parse_unary(Parser *parser) {
    Token token;
    ASTNode *node;
    ASTNode *right;

    token = parser->current;

    if (parser_match(parser, TOKEN_NOT) || parser_match(parser, TOKEN_MINUS)) {
        node = create_node(NODE_UNARY, token.lexeme, token.line);
        right = parse_unary(parser);
        add_child(node, right);

        return node;
    }

    return parse_primary(parser);
}

static ASTNode *parse_factor(Parser *parser) {
    ASTNode *node;
    ASTNode *right;
    ASTNode *parent;
    Token token;

    node = parse_unary(parser);

    while (parser->current.type == TOKEN_STAR || parser->current.type == TOKEN_SLASH) {
        token = parser->current;
        parser_advance(parser);

        right = parse_unary(parser);
        parent = create_node(NODE_BINARY, token.lexeme, token.line);

        add_child(parent, node);
        add_child(parent, right);

        node = parent;
    }

    return node;
}

static ASTNode *parse_term(Parser *parser) {
    ASTNode *node;
    ASTNode *right;
    ASTNode *parent;
    Token token;

    node = parse_factor(parser);

    while (parser->current.type == TOKEN_PLUS || parser->current.type == TOKEN_MINUS) {
        token = parser->current;
        parser_advance(parser);

        right = parse_factor(parser);
        parent = create_node(NODE_BINARY, token.lexeme, token.line);

        add_child(parent, node);
        add_child(parent, right);

        node = parent;
    }

    return node;
}

static ASTNode *parse_comparison(Parser *parser) {
    ASTNode *node;
    ASTNode *right;
    ASTNode *parent;
    Token token;

    node = parse_term(parser);

    while (
        parser->current.type == TOKEN_LESS ||
        parser->current.type == TOKEN_LESS_EQUAL ||
        parser->current.type == TOKEN_GREATER ||
        parser->current.type == TOKEN_GREATER_EQUAL
    ) {
        token = parser->current;
        parser_advance(parser);

        right = parse_term(parser);
        parent = create_node(NODE_BINARY, token.lexeme, token.line);

        add_child(parent, node);
        add_child(parent, right);

        node = parent;
    }

    return node;
}

static ASTNode *parse_equality(Parser *parser) {
    ASTNode *node;
    ASTNode *right;
    ASTNode *parent;
    Token token;

    node = parse_comparison(parser);

    while (parser->current.type == TOKEN_EQUAL || parser->current.type == TOKEN_NOT_EQUAL) {
        token = parser->current;
        parser_advance(parser);

        right = parse_comparison(parser);
        parent = create_node(NODE_BINARY, token.lexeme, token.line);

        add_child(parent, node);
        add_child(parent, right);

        node = parent;
    }

    return node;
}

static ASTNode *parse_logic_and(Parser *parser) {
    ASTNode *node;
    ASTNode *right;
    ASTNode *parent;
    Token token;

    node = parse_equality(parser);

    while (parser->current.type == TOKEN_AND) {
        token = parser->current;
        parser_advance(parser);

        right = parse_equality(parser);
        parent = create_node(NODE_BINARY, token.lexeme, token.line);

        add_child(parent, node);
        add_child(parent, right);

        node = parent;
    }

    return node;
}

static ASTNode *parse_expression(Parser *parser) {
    ASTNode *node;
    ASTNode *right;
    ASTNode *parent;
    Token token;

    node = parse_logic_and(parser);

    while (parser->current.type == TOKEN_OR) {
        token = parser->current;
        parser_advance(parser);

        right = parse_logic_and(parser);
        parent = create_node(NODE_BINARY, token.lexeme, token.line);

        add_child(parent, node);
        add_child(parent, right);

        node = parent;
    }

    return node;
}

static ASTNode *parse_assignment_core(Parser *parser, int need_semicolon) {
    Token id_token;
    ASTNode *node;
    ASTNode *id_node;
    ASTNode *expr;

    id_token = parser->current;

    parser_expect(parser, TOKEN_ID, "se esperaba identificador en asignacion");
    parser_expect(parser, TOKEN_ASSIGN, "se esperaba '=' en asignacion");

    expr = parse_expression(parser);

    node = create_node(NODE_ASSIGNMENT, "asignacion", id_token.line);
    id_node = create_node(NODE_IDENTIFIER, id_token.lexeme, id_token.line);

    add_child(node, id_node);
    add_child(node, expr);

    if (need_semicolon == 1) {
        parser_expect(parser, TOKEN_SEMICOLON, "se esperaba ';' al cerrar asignacion");
    }

    return node;
}

static ASTNode *parse_declaration(Parser *parser) {
    DataType type;
    Token id_token;
    ASTNode *node;
    ASTNode *expr;

    type = parse_type(parser);

    id_token = parser->current;

    parser_expect(parser, TOKEN_ID, "se esperaba identificador en declaracion");

    node = create_node(NODE_DECLARATION, id_token.lexeme, id_token.line);
    node->data_type = type;

    if (parser_match(parser, TOKEN_ASSIGN)) {
        expr = parse_expression(parser);
        add_child(node, expr);
    }

    parser_expect(parser, TOKEN_SEMICOLON, "se esperaba ';' al cerrar declaracion");

    return node;
}

static ASTNode *parse_if(Parser *parser) {
    ASTNode *node;
    ASTNode *condition;
    ASTNode *then_block;
    ASTNode *else_part;
    Token token;

    token = parser->current;

    parser_expect(parser, TOKEN_IF, "se esperaba if");
    parser_expect(parser, TOKEN_LPAREN, "se esperaba '(' despues de if");

    condition = parse_expression(parser);

    parser_expect(parser, TOKEN_RPAREN, "se esperaba ')' despues de condicion");

    enter_control(parser);
    then_block = parse_block(parser);
    exit_control(parser);

    node = create_node(NODE_IF, "if", token.line);

    add_child(node, condition);
    add_child(node, then_block);

    if (parser_match(parser, TOKEN_ELSE)) {
        if (parser->current.type == TOKEN_IF) {
            else_part = parse_if(parser);
            add_child(node, else_part);
        } else {
            enter_control(parser);
            else_part = parse_block(parser);
            exit_control(parser);

            add_child(node, else_part);
        }
    }

    return node;
}

static ASTNode *parse_while(Parser *parser) {
    ASTNode *node;
    ASTNode *condition;
    ASTNode *body;
    Token token;

    token = parser->current;

    parser_expect(parser, TOKEN_WHILE, "se esperaba while");
    parser_expect(parser, TOKEN_LPAREN, "se esperaba '(' despues de while");

    condition = parse_expression(parser);

    parser_expect(parser, TOKEN_RPAREN, "se esperaba ')' despues de condicion");

    enter_control(parser);
    body = parse_block(parser);
    exit_control(parser);

    node = create_node(NODE_WHILE, "while", token.line);

    add_child(node, condition);
    add_child(node, body);

    return node;
}

static ASTNode *parse_do_while(Parser *parser) {
    ASTNode *node;
    ASTNode *body;
    ASTNode *condition;
    Token token;

    token = parser->current;

    parser_expect(parser, TOKEN_DO, "se esperaba do");

    enter_control(parser);
    body = parse_block(parser);
    exit_control(parser);

    parser_expect(parser, TOKEN_WHILE, "se esperaba while despues del bloque do");
    parser_expect(parser, TOKEN_LPAREN, "se esperaba '(' despues de while");

    condition = parse_expression(parser);

    parser_expect(parser, TOKEN_RPAREN, "se esperaba ')' despues de condicion");
    parser_expect(parser, TOKEN_SEMICOLON, "se esperaba ';' al cerrar do-while");

    node = create_node(NODE_DO_WHILE, "do-while", token.line);

    add_child(node, body);
    add_child(node, condition);

    return node;
}

static ASTNode *parse_for(Parser *parser) {
    ASTNode *node;
    ASTNode *init;
    ASTNode *condition;
    ASTNode *increment;
    ASTNode *body;
    Token token;

    token = parser->current;

    parser_expect(parser, TOKEN_FOR, "se esperaba for");
    parser_expect(parser, TOKEN_LPAREN, "se esperaba '(' despues de for");

    init = parse_assignment_core(parser, 0);

    parser_expect(parser, TOKEN_SEMICOLON, "se esperaba ';' despues de inicializacion del for");

    condition = parse_expression(parser);

    parser_expect(parser, TOKEN_SEMICOLON, "se esperaba ';' despues de condicion del for");

    increment = parse_assignment_core(parser, 0);

    parser_expect(parser, TOKEN_RPAREN, "se esperaba ')' al cerrar for");

    enter_control(parser);
    body = parse_block(parser);
    exit_control(parser);

    node = create_node(NODE_FOR, "for", token.line);

    add_child(node, init);
    add_child(node, condition);
    add_child(node, increment);
    add_child(node, body);

    return node;
}

static ASTNode *parse_statement(Parser *parser) {
    if (token_is_type(parser->current.type) == 1) {
        return parse_declaration(parser);
    }

    if (parser->current.type == TOKEN_ID) {
        return parse_assignment_core(parser, 1);
    }

    if (parser->current.type == TOKEN_IF) {
        return parse_if(parser);
    }

    if (parser->current.type == TOKEN_WHILE) {
        return parse_while(parser);
    }

    if (parser->current.type == TOKEN_DO) {
        return parse_do_while(parser);
    }

    if (parser->current.type == TOKEN_FOR) {
        return parse_for(parser);
    }

    print_error("Sintactico", parser->current.line, parser->current.column, "sentencia no reconocida");

    parser->has_error = 1;

    parser_advance(parser);

    return create_node(NODE_EMPTY, "sentencia_error", parser->current.line);
}

static ASTNode *parse_block(Parser *parser) {
    ASTNode *block;
    ASTNode *statement;

    block = create_node(NODE_BLOCK, "bloque", parser->current.line);

    parser_expect(parser, TOKEN_LBRACE, "se esperaba '{' al abrir bloque");

    while (parser->current.type != TOKEN_RBRACE && parser->current.type != TOKEN_EOF) {
        statement = parse_statement(parser);
        add_child(block, statement);
    }

    parser_expect(parser, TOKEN_RBRACE, "se esperaba '}' al cerrar bloque");

    return block;
}

ASTNode *parse_program(Parser *parser) {
    ASTNode *program;
    ASTNode *body;
    Token id_token;

    parser_expect(parser, TOKEN_PROGRAM, "el archivo debe iniciar con 'program'");

    id_token = parser->current;

    parser_expect(parser, TOKEN_ID, "se esperaba nombre del programa");

    body = parse_block(parser);

    parser_expect(parser, TOKEN_EOF, "no debe existir codigo despues del cierre del programa");

    program = create_node(NODE_PROGRAM, id_token.lexeme, id_token.line);

    add_child(program, body);

    return program;
}