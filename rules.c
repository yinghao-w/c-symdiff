#include "symbols.h"
#include "ast.h"
#include <complex.h>
#include "rules.h"

#define NUM_RULES 2

struct rule {
	tok_Node *pattern;
	tok_Node *replacement;
};

struct rule rules[NUM_RULES];

struct bindings {
	tok_Node *f_bind;
	tok_Node *g_bind;
	int flag;
};

static struct bindings set(struct bindings binds, Var var, tok_Node *expr) {
	if (var == 'f') {
		binds.f_bind = expr;
	} else {
		binds.g_bind = expr;
	}
	binds.flag = 1;
	return binds;
}

static tok_Node *get(struct bindings binds, Var var) {
	if (binds.flag == 0) {
		return NULL;
	} else if (var == 'f') {
		return binds.f_bind;
	}
	return binds.g_bind;
}

static struct bindings join(struct bindings bind1, struct bindings bind2) {
	struct bindings binds;
	if (bind1.f_bind) {
		binds.f_bind = bind1.f_bind;
	} else {
		binds.f_bind = bind2.f_bind;
	}
	if (bind1.g_bind) {
		binds.g_bind = bind1.g_bind;
	} else {
		binds.g_bind = bind2.g_bind;
	}
	binds.flag = 1;
	return binds;
}

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

/* a recursive deep copy function. Using recursion because the target are the
 * replacement trees in the rule structs, which are small. */
static tok_Node *r_copy(const tok_Node *node) {
	if (node == NULL) {
		return NULL;
	}
	return tok_join(node->value, r_copy(node->lchild),r_copy(node->rchild));
}

struct bindings match(tok_Node *expr, tok_Node *pattern) {
	struct bindings binds = {.f_bind = NULL, .g_bind=NULL, .flag = -1};
	if (t_is_leaf(pattern)) {
		if (pattern->value.token_type == SCALAR) {
			if ((expr->value.token_type != SCALAR) || (expr->value.scalar != pattern->value.scalar)) {
				binds.flag = 0;
			}
			binds.flag = 1;
		} else {
			binds = set(binds, pattern->value.var, expr);
			// bind here
		}
		return binds;
	}
	struct bindings l_binds = match(expr->lchild, pattern->lchild);
	struct bindings r_binds = match(expr->rchild, pattern->rchild);
	if (!l_binds.flag || !r_binds.flag) {
		binds.flag = 0;
		return binds;
	}
	binds = join(l_binds, r_binds);
	return binds;
}

void rule_apply(tok_Node *expr, struct rule rule) {
	struct bindings binds = match(expr, rule.pattern);
	tok_disconnectc(bindings);

	// match expr and rule.pattern, and generate a set of bindings, if any

	tok_Node* new_expr = r_copy(rule.replacement);
	foreach(tok_Node, sub_expr, new_expr) {
		if (1) {
			ast_sub(sub_expr, get(bindings, sub_expr->value.var));
			tok_destroy(sub_expr);
		}
	}


	
}
