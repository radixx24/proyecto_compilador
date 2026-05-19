#include <stdio.h>
#include <string.h>

#include "semantic.h"
#include "symbol_table.h"

static int is_numeric(DataType type) {
    if (type == TYPE_INT || type == TYPE_FLOAT) {
        return 1;
    }

    return 0;
}

static int types_compatible(DataType expected, DataType received) {
    if (expected == received) {
        return 1;
    }

    if (expected == TYPE_FLOAT && received == TYPE_INT) {
        return 1;
    }

    return 0;
}

static DataType semantic_expression(ASTNode *node, SymbolTable *table, int *errors);

static DataType semantic_binary(ASTNode *node, SymbolTable *table, int *errors) {
    DataType left;
    DataType right;

    left = semantic_expression(node->children[0], table, errors);
    right = semantic_expression(node->children[1], table, errors);

    if (
        strcmp(node->value, "+") == 0 ||
        strcmp(node->value, "-") == 0 ||
        strcmp(node->value, "*") == 0 ||
        strcmp(node->value, "/") == 0
    ) {
        if (is_numeric(left) == 1 && is_numeric(right) == 1) {
            if (left == TYPE_FLOAT || right == TYPE_FLOAT) {
                node->data_type = TYPE_FLOAT;
                return TYPE_FLOAT;
            }

            node->data_type = TYPE_INT;
            return TYPE_INT;
        }

        printf("[Semantico] Error en linea %d: operador '%s' requiere operandos numericos.\n", node->line, node->value);

        *errors = *errors + 1;

        return TYPE_ERROR;
    }

    if (
        strcmp(node->value, "<") == 0 ||
        strcmp(node->value, "<=") == 0 ||
        strcmp(node->value, ">") == 0 ||
        strcmp(node->value, ">=") == 0
    ) {
        if (is_numeric(left) == 1 && is_numeric(right) == 1) {
            node->data_type = TYPE_BOOL;
            return TYPE_BOOL;
        }

        printf("[Semantico] Error en linea %d: comparacion '%s' requiere operandos numericos.\n", node->line, node->value);

        *errors = *errors + 1;

        return TYPE_ERROR;
    }

    if (strcmp(node->value, "==") == 0 || strcmp(node->value, "!=") == 0) {
        if (types_compatible(left, right) == 1 || types_compatible(right, left) == 1) {
            node->data_type = TYPE_BOOL;
            return TYPE_BOOL;
        }

        printf(
            "[Semantico] Error en linea %d: igualdad entre tipos incompatibles '%s' y '%s'.\n",
            node->line,
            type_to_string(left),
            type_to_string(right)
        );

        *errors = *errors + 1;

        return TYPE_ERROR;
    }

    if (strcmp(node->value, "&&") == 0 || strcmp(node->value, "||") == 0) {
        if (left == TYPE_BOOL && right == TYPE_BOOL) {
            node->data_type = TYPE_BOOL;
            return TYPE_BOOL;
        }

        printf("[Semantico] Error en linea %d: operador logico '%s' requiere booleanos.\n", node->line, node->value);

        *errors = *errors + 1;

        return TYPE_ERROR;
    }

    return TYPE_ERROR;
}

static DataType semantic_expression(ASTNode *node, SymbolTable *table, int *errors) {
    Symbol *symbol;
    DataType child_type;

    if (node == NULL) {
        return TYPE_VOID;
    }

    if (node->node_type == NODE_LITERAL) {
        return node->data_type;
    }

    if (node->node_type == NODE_IDENTIFIER) {
        symbol = symbol_lookup(table, node->value);

        if (symbol == NULL) {
            printf("[Semantico] Error en linea %d: variable '%s' no declarada.\n", node->line, node->value);

            *errors = *errors + 1;

            return TYPE_ERROR;
        }

        node->data_type = symbol->type;

        return symbol->type;
    }

    if (node->node_type == NODE_BINARY) {
        return semantic_binary(node, table, errors);
    }

    if (node->node_type == NODE_UNARY) {
        child_type = semantic_expression(node->children[0], table, errors);

        if (strcmp(node->value, "!") == 0) {
            if (child_type == TYPE_BOOL) {
                node->data_type = TYPE_BOOL;
                return TYPE_BOOL;
            }

            printf("[Semantico] Error en linea %d: operador ! requiere booleano.\n", node->line);

            *errors = *errors + 1;

            return TYPE_ERROR;
        }

        if (strcmp(node->value, "-") == 0) {
            if (is_numeric(child_type) == 1) {
                node->data_type = child_type;
                return child_type;
            }

            printf("[Semantico] Error en linea %d: operador - requiere numero.\n", node->line);

            *errors = *errors + 1;

            return TYPE_ERROR;
        }
    }

    return TYPE_ERROR;
}

static void semantic_statement(ASTNode *node, SymbolTable *table, int *errors);

static void semantic_block(ASTNode *node, SymbolTable *table, int *errors) {
    int i;

    symbol_enter_scope(table);

    for (i = 0; i < node->child_count; i = i + 1) {
        semantic_statement(node->children[i], table, errors);
    }

    symbol_exit_scope(table);
}

static void semantic_declaration(ASTNode *node, SymbolTable *table, int *errors) {
    DataType expr_type;
    int added;

    added = symbol_add(table, node->value, node->data_type, node->line);

    if (added == 0) {
        *errors = *errors + 1;
    }

    if (node->child_count == 1) {
        expr_type = semantic_expression(node->children[0], table, errors);

        if (types_compatible(node->data_type, expr_type) == 0) {
            printf(
                "[Semantico] Error en linea %d: no se puede inicializar '%s' con tipo '%s'.\n",
                node->line,
                node->value,
                type_to_string(expr_type)
            );

            *errors = *errors + 1;
        }
    }
}

static void semantic_assignment(ASTNode *node, SymbolTable *table, int *errors) {
    Symbol *symbol;
    DataType expr_type;
    ASTNode *id_node;

    id_node = node->children[0];
    symbol = symbol_lookup(table, id_node->value);

    if (symbol == NULL) {
        printf("[Semantico] Error en linea %d: variable '%s' no declarada.\n", id_node->line, id_node->value);

        *errors = *errors + 1;

        return;
    }

    expr_type = semantic_expression(node->children[1], table, errors);

    if (types_compatible(symbol->type, expr_type) == 0) {
        printf(
            "[Semantico] Error en linea %d: asignacion incompatible. Variable '%s' es '%s', expresion es '%s'.\n",
            node->line,
            id_node->value,
            type_to_string(symbol->type),
            type_to_string(expr_type)
        );

        *errors = *errors + 1;
    }
}

static void semantic_condition(ASTNode *condition, SymbolTable *table, int *errors, const char *control_name) {
    DataType type;

    type = semantic_expression(condition, table, errors);

    if (type != TYPE_BOOL) {
        printf(
            "[Semantico] Error en linea %d: la condicion de '%s' debe ser bool.\n",
            condition->line,
            control_name
        );

        *errors = *errors + 1;
    }
}

static void semantic_statement(ASTNode *node, SymbolTable *table, int *errors) {
    if (node == NULL) {
        return;
    }

    if (node->node_type == NODE_DECLARATION) {
        semantic_declaration(node, table, errors);
        return;
    }

    if (node->node_type == NODE_ASSIGNMENT) {
        semantic_assignment(node, table, errors);
        return;
    }

    if (node->node_type == NODE_BLOCK) {
        semantic_block(node, table, errors);
        return;
    }

    if (node->node_type == NODE_IF) {
        semantic_condition(node->children[0], table, errors, "if");
        semantic_block(node->children[1], table, errors);

        if (node->child_count == 3) {
            semantic_statement(node->children[2], table, errors);
        }

        return;
    }

    if (node->node_type == NODE_WHILE) {
        semantic_condition(node->children[0], table, errors, "while");
        semantic_block(node->children[1], table, errors);

        return;
    }

    if (node->node_type == NODE_DO_WHILE) {
        semantic_block(node->children[0], table, errors);
        semantic_condition(node->children[1], table, errors, "do-while");

        return;
    }

    if (node->node_type == NODE_FOR) {
        symbol_enter_scope(table);

        semantic_assignment(node->children[0], table, errors);
        semantic_condition(node->children[1], table, errors, "for");
        semantic_assignment(node->children[2], table, errors);
        semantic_block(node->children[3], table, errors);

        symbol_exit_scope(table);

        return;
    }
}

int semantic_analyze(ASTNode *program) {
    SymbolTable table;
    int errors;

    errors = 0;

    symbol_table_init(&table);

    if (program != NULL && program->child_count > 0) {
        semantic_block(program->children[0], &table, &errors);
    }

    return errors;
}