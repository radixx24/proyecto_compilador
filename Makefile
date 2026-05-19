CC=gcc
CFLAGS=-std=c11 -Wall -Wextra -pedantic -Iinclude

TARGET=compiler_lp

SOURCES=src/main.c \
	src/lexer.c \
	src/parser.c \
	src/ast.c \
	src/semantic.c \
	src/symbol_table.c \
	src/utils.c

all:
	$(CC) $(CFLAGS) $(SOURCES) -o $(TARGET)

run:
	./$(TARGET) tests/valido.lp

tokens:
	./$(TARGET) --tokens tests/valido.lp

test-valid:
	./$(TARGET) tests/valido.lp

test-type:
	./$(TARGET) tests/error_tipo.lp

test-variable:
	./$(TARGET) tests/error_variable_no_declarada.lp

test-depth:
	./$(TARGET) tests/error_anidamiento.lp

clean:
	rm -f $(TARGET)