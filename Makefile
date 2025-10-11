CC = gcc
CFLAGS = -std=c11 -g -Wall -Wextra -Wpedantic -Wno-unused-function -Wno-return-type -lm
CMATH = -lm
INCLUDE = -I ../c-generics
OUTPUT = main

all:
	@$(CC) $(CFLAGS) $(INCLUDE) -o $(OUTPUT).out main.c transforms.c ast.c lexer.c symbols.c $(CMATH)
	./$(OUTPUT).out

run:
	./$(OUTPUT).out

clean:
	rm $(OUTPUT).out

