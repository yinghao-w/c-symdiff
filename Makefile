CC = gcc
CFLAGS = -std=c11 -g -Wall -Wextra -Wpedantic -Wno-unused-function -Wno-return-type -lm
CMATH = -lm
INCLUDE = -I ../c-generics
OUTPUT = main

all:
	@$(CC) $(CFLAGS) $(INCLUDE) -o $(OUTPUT).out main.c lexer.c symbols.c $(CMATH)

clean:
	rm $(OUTPUT).out

