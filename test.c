#include "symbols.h"
#include <stdio.h>
#include "ast.h"

void tok_print(tok_Node *node) {
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

	char r[] = "( x + y ) - ( c - 2 ) * 5";
	char w[] = "sq ( 2 + x ) ^ 3";
	char q[] = "( x * 2 ) D x";
	char s[] = "a";
	tok_Node *t = shunting_yard("1 + 2 / 3");

	foreach(tok_Node, node, t) {
		printf("Node: ");
		tok_print(node);
		printf("\tParent: ");
		node->parent ? tok_print(node->parent) : 0;
		printf("\n");
	}

	tok_destroy(t);
}

int main(void) {
	test1();
	return 0;
}
