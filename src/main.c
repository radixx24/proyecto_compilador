#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "parser.h"
#include "semantic.h"
#include "utils.h"

static int run_compiler(const char *path, int show_tokens) {
    char *source;
    Parser parser;
    ASTNode *program;
    int semantic_errors;

    if (has_lp_extension(path) == 0) {
        printf("Error: el archivo debe tener extension .lp por los apellidos Lopez Palacios.\n");
        return 1;
    }

    source = read_file(path);

    if (source == NULL) {
        printf("Error: no se pudo leer el archivo '%s'.\n", path);
        return 1;
    }

    if (show_tokens == 1) {
        printf("\nTokens generados:\n");
        print_all_tokens(source);
        printf("\n");
    }

    parser_init(&parser, source);

    program = parse_program(&parser);

    if (parser.has_error == 1) {
        printf("\nResultado: el analisis lexico/sintactico fallo.\n");

        free_ast(program);
        free(source);

        return 1;
    }

    semantic_errors = semantic_analyze(program);

    printf("\nAST generado:\n");
    print_ast(program, 0);

    printf("\nProfundidad maxima de estructuras de control: %d\n", parser.max_control_depth);

    if (semantic_errors > 0) {
        printf("\nResultado: el analisis semantico fallo con %d error(es).\n", semantic_errors);

        free_ast(program);
        free(source);

        return 1;
    }

    printf("\nResultado: archivo valido. Front-End completado correctamente.\n");

    free_ast(program);
    free(source);

    return 0;
}

int main(int argc, char **argv) {
    int show_tokens;
    const char *path;

    show_tokens = 0;
    path = NULL;

    if (argc == 2) {
        path = argv[1];
    } else if (argc == 3) {
        if (strcmp(argv[1], "--tokens") == 0) {
            show_tokens = 1;
            path = argv[2];
        } else {
            printf("Uso: %s [--tokens] archivo.lp\n", argv[0]);
            return 1;
        }
    } else {
        printf("Uso: %s [--tokens] archivo.lp\n", argv[0]);
        return 1;
    }

    return run_compiler(path, show_tokens);
}