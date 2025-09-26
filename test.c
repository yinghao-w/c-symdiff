#include "symbols.h"
#include <stdio.h>
#include "ast.h"

void ast_print(tok_Node *node) {
	switch (node->value.token_type) {
	case OPR:
		printf("%s", (*(node->value.opr)).repr);
		break;
	case SCALAR:
		printf("%.1f", node->value.scalar);
		break;
	case VAR:
		printf("%c", node->value.var);
		break;
	}
}

void test1(void) {
	int a = get_opr("-")->arity;
	printf("%d\n", a);
	Opr *o1 = get_opr("+");
	Opr *o2 = get_opr("*");
	int pre = oprcmp(o1, o2);
	printf("%d\n\n\n", pre);

	char q[] = "( x + y ) - ( c - 2 ) * 5";
	char w[] = "sq ( 2 + x ) ^ 3";
	char s[] = "2 -3";
	tok_Node *t = shunting_yard(s);

	foreach(tok_Node, node, t) {
		printf("Node: ");
		ast_print(node);
		printf("\tParent: ");
		node->parent ? ast_print(node->parent) : 0;
		printf("\n");
	}
}

int main(void) {
	test1();
	return 0;
}
