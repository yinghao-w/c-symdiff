#include "symbols.h"
#include "ast.h"
#include "rules.h"

#define NUM_RULES 2

struct rule {
	tok_Node *pattern;
	tok_Node *replacement;
};

struct rule rules[NUM_RULES];

/* TODO: Store all nodes of trees in rules in static memory, not heap. */

void rule_init(void) {

	struct rule rule_add_0 = {
		.pattern = shunting_yard("f + 0"),
		.replacement = shunting_yard("f"),
	};
	struct rule rule_mult_0 = {
		.pattern = shunting_yard("f * 0"),
		.replacement = shunting_yard("0"),
	};

	rules[0] = rule_add_0;
	rules[1] = rule_mult_0;
}

int match(tok_Node *node, tok_Node *pattern) {
	if (pattern == NULL) {
		return 1;
	} else if (t_is_leaf(pattern)) {
		if (pattern->value.token_type == VAR) {
			return 1;
		} else if (pattern->value.token_type == SCALAR) {
			return node->value.scalar ==pattern->value.scalar;
		}
	}
	int curr_match = (node->value.opr == pattern->value.opr);
	return (match(node->lchild, pattern->lchild) && curr_match && match(node->rchild, pattern->rchild));
}

void rule_apply(tok_Node *node, )
