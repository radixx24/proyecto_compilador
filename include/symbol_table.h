#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include "ast.h"

#define MAX_SYMBOLS 512

typedef struct {
    char name[MAX_LEXEME];
    DataType type;
    int scope_level;
    int line_declared;
} Symbol;

typedef struct {
    Symbol symbols[MAX_SYMBOLS];
    int count;
    int scope_level;
} SymbolTable;

void symbol_table_init(SymbolTable *table);
void symbol_enter_scope(SymbolTable *table);
void symbol_exit_scope(SymbolTable *table);
Symbol *symbol_lookup(SymbolTable *table, const char *name);
Symbol *symbol_lookup_current_scope(SymbolTable *table, const char *name);
int symbol_add(SymbolTable *table, const char *name, DataType type, int line);

#endif