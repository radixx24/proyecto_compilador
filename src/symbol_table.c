#include <stdio.h>
#include <string.h>

#include "symbol_table.h"

void symbol_table_init(SymbolTable *table) {
    table->count = 0;
    table->scope_level = 0;
}

void symbol_enter_scope(SymbolTable *table) {
    table->scope_level = table->scope_level + 1;
}

void symbol_exit_scope(SymbolTable *table) {
    int i;
    int write_index;

    write_index = 0;

    for (i = 0; i < table->count; i = i + 1) {
        if (table->symbols[i].scope_level < table->scope_level) {
            table->symbols[write_index] = table->symbols[i];
            write_index = write_index + 1;
        }
    }

    table->count = write_index;

    if (table->scope_level > 0) {
        table->scope_level = table->scope_level - 1;
    }
}

Symbol *symbol_lookup(SymbolTable *table, const char *name) {
    int i;

    for (i = table->count - 1; i >= 0; i = i - 1) {
        if (strcmp(table->symbols[i].name, name) == 0) {
            return &table->symbols[i];
        }
    }

    return NULL;
}

Symbol *symbol_lookup_current_scope(SymbolTable *table, const char *name) {
    int i;

    for (i = table->count - 1; i >= 0; i = i - 1) {
        if (strcmp(table->symbols[i].name, name) == 0 && table->symbols[i].scope_level == table->scope_level) {
            return &table->symbols[i];
        }
    }

    return NULL;
}

int symbol_add(SymbolTable *table, const char *name, DataType type, int line) {
    Symbol *existing;

    existing = symbol_lookup_current_scope(table, name);

    if (existing != NULL) {
        printf("[Semantico] Error en linea %d: variable '%s' ya declarada en este ambito.\n", line, name);
        return 0;
    }

    if (table->count >= MAX_SYMBOLS) {
        printf("[Semantico] Error: tabla de simbolos llena.\n");
        return 0;
    }

    strncpy(table->symbols[table->count].name, name, MAX_LEXEME - 1);
    table->symbols[table->count].name[MAX_LEXEME - 1] = '\0';

    table->symbols[table->count].type = type;
    table->symbols[table->count].scope_level = table->scope_level;
    table->symbols[table->count].line_declared = line;

    table->count = table->count + 1;

    return 1;
}