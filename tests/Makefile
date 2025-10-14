CC = gcc
CFLAGS = -std=c11 -g -Wall -Wextra -Wpedantic -Wno-unused-function -Wno-return-type -lm
CMATH = -lm
INCLUDE = -I ..
TESTS = tree_test symbols_test lexer_test ast_test

all: $(TESTS)
	@$(foreach f, $^, ./$(f).out;)

clean:
		rm -fr *.out

run:
	@$(foreach f, $(TESTS), ./$(f).out;)

tree_test:
	@$(CC) $(CFLAGS) $(INCLUDE) -o $@.out $@.c

symbols_test: 
	@$(CC) $(CFLAGS) $(INCLUDE) -o $@.out $@.c $(CMATH)

lexer_test: 
	@$(CC) $(CFLAGS) $(INCLUDE) -o $@.out $@.c ../symbols.c $(CMATH)

ast_test:
	@$(CC) $(CFLAGS) $(INCLUDE) -o $@.out $@.c ../symbols.c ../lexer.c $(CMATH)
