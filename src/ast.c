#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"

const char *type_to_string(DataType type) {
    if (type == TYPE_INT) {
        return "int";
    }

    if (type == TYPE_FLOAT) {
        return "float";
    }

    if (type == TYPE_BOOL) {
        return "bool";
    }

    if (type == TYPE_STRING) {
        return "string";
    }

    if (type == TYPE_VOID) {
        return "void";
    }

    return "error";
}

ASTNode *create_node(NodeType node_type, const char *value, int line) {
    ASTNode *node;
    int i;

    node = (ASTNode *)malloc(sizeof(ASTNode));

    if (node == NULL) {
        printf("Error: no se pudo reservar memoria para AST.\n");
        exit(1);
    }

    node->node_type = node_type;
    node->data_type = TYPE_VOID;

    strncpy(node->value, value, MAX_LEXEME - 1);
    node->value[MAX_LEXEME - 1] = '\0';

    node->line = line;
    node->child_count = 0;

    for (i = 0; i < MAX_CHILDREN; i = i + 1) {
        node->children[i] = NULL;
    }

    return node;
}

void add_child(ASTNode *parent, ASTNode *child) {
    if (parent == NULL || child == NULL) {
        return;
    }

    if (parent->child_count >= MAX_CHILDREN) {
        printf("Error: demasiados hijos en nodo AST.\n");
        exit(1);
    }

    parent->children[parent->child_count] = child;
    parent->child_count = parent->child_count + 1;
}

void free_ast(ASTNode *node) {
    int i;

    if (node == NULL) {
        return;
    }

    for (i = 0; i < node->child_count; i = i + 1) {
        free_ast(node->children[i]);
    }

    free(node);
}

void print_ast(ASTNode *node, int level) {
    int i;

    if (node == NULL) {
        return;
    }

    for (i = 0; i < level; i = i + 1) {
        printf("  ");
    }

    printf("%s", node->value);

    if (node->data_type != TYPE_VOID) {
        printf(" : %s", type_to_string(node->data_type));
    }

    printf("\n");

    for (i = 0; i < node->child_count; i = i + 1) {
        print_ast(node->children[i], level + 1);
    }
}