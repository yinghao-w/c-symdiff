#define T_DEBUG
#define SYMBOLS_DEBUG
#include "ast.h"
#include "symbols.h"
#include "transforms.h"
#include <stdio.h>

int main(void) {
	opr_set_init();
	simpls_init();
	rules_init();
	diff_rules_init();

	Expression *expr = expr_create("x'3 + 4");
	ast_DEBUG_PRINT(expr->ast_tree, token_print);
	printf("\n");
	diff_apply(expr);
	ast_DEBUG_PRINT(expr->ast_tree, token_print);

	printf("asdjijasdkjdas\n");
	return 0;
}
