CFLAGS = -std=c11 -g -Wall -Wextra -Wpedantic -Wno-unused-function -Wno-return-type
CMATH = -lm
INCLUDE = -I .
OUTPUT = main
TEST_DIR = tests
TESTS = tree_test symbols_test lexer_test ast_test

all:
	@$(CC) $(CFLAGS) $(INCLUDE) -o $(OUTPUT).out main.c lexer.c symbols.c $(CMATH)

tests: $(TESTS) run-tests

run-tests:
	@$(foreach f, $(TESTS), ./$(TEST_DIR)/$(f).out;)

tree_test:
	@$(CC) $(CFLAGS) $(INCLUDE) -o $(TEST_DIR)/$@.out $(TEST_DIR)/$@.c

symbols_test: 
	@$(CC) $(CFLAGS) $(INCLUDE) -o $(TEST_DIR)/$@.out $(TEST_DIR)/$@.c $(CMATH)

lexer_test: 
	@$(CC) $(CFLAGS) $(INCLUDE) -o $(TEST_DIR)/$@.out $(TEST_DIR)/$@.c symbols.c $(CMATH)

ast_test:
	@$(CC) $(CFLAGS) $(INCLUDE) -o $(TEST_DIR)/$@.out $(TEST_DIR)/$@.c symbols.c lexer.c $(CMATH)

clean:
	rm *.out $(TEST_DIR)/*.out

