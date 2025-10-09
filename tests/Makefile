CC = gcc
CFLAGS = -std=c11 -g -Wall -Wextra -Wpedantic -Wno-unused-function
TESTS = tree_test symbols_test lexer_test


all: $(TESTS)
	$(foreach f, $^, ./$(f).out;)

clean:
		rm -fr *.out

tree_test:
	$(CC) $(CFLAGS) -o $@.out tree_test.c

symbols_test: 
	$(CC) $(CFLAGS) -o $@.out symbols_test.c

lexer_test: 
	$(CC) $(CFLAGS) -o $@.out lexer_test.c ../symbols.c

