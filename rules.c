#include "symbols.h"
#include "ast.h"
#include <bits/types/siginfo_t.h>
#include <complex.h>
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

/* a recursive deep copy function. Using recursion because the arguments will
 * be the replacement trees in the rule structs, which are small. */
static tok_Node *r_copy(const tok_Node *node) {
	if (node == NULL) {
		return NULL;
	}
	return tok_join(node->value, r_copy(node->lchild),r_copy(node->rchild));
}


static int tokcmp(const Token token1, const Token token2) {
	if (token1.token_type == token2.token_type) {
		switch (token1.token_type) {
		case SCALAR:
			return token1.scalar == token2.scalar;
			break;
		case VAR:
			return token1.var == token2.var;
			break;
		case OPR:
			return token1.opr == token2.opr;
			break;
		}
	} else {
		return 0;
	}
}


struct binding {
	Var var;
	tok_Node *expr;
};

struct bindings {
	struct binding bind1;
	struct binding bind2;
};

typedef struct bindings hashmap; // pretend its a hashmap/table
								 //
hashmap *hm_init(void);

int is_bound(Var var, hashmap *hm);

tok_Node *bind_get(Var var);

void bind_set(Var var, tok_Node *expr, hashmap *hm);

int match(tok_Node *expr, tok_Node *pattern, hashmap *hm) {


	if (pattern == NULL) {
		return 1;
	} else if (expr == NULL) {
		/* will not work for optional bindings, e.g. variadic functions */
		return 0;
	} else if (t_is_leaf(pattern)) {
		if (pattern->value.token_type == SCALAR) {
			/* if expr does not match the scalar pattern: */
			if ((expr->value.token_type != SCALAR) || (expr->value.scalar != pattern->value.scalar)) {
				return 0;
			}
		/* if pattern is a variable, will match anything non-NULL */
		} else {
			if (is_bound(pattern->value.var, hm)) {
				return tok_is_equal(expr, bind_get(pattern->value.var), tokcmp);
			} else {
				bind_set(pattern->value.var, expr, hm);
			}
		}
		/* if pattern is variable or scalar which matches expr: */
		return 1;
	} else if (expr->value.opr != pattern->value.opr) {
		return 0;
	}
	return match(expr->lchild, pattern->lchild, hm) &&  match(expr->rchild, pattern->rchild, hm);
}



void rule_apply(tok_Node *expr, struct rule rule) {
	
	hashmap *hm = hm_init();
	match(expr, rule.pattern, hm);

	// for var, expr in hm:
	// 		tok_detach(expr)

	tok_detach(expr);

	// match expr and rule.pattern, and generate a set of bindings, if any

	tok_Node* new_expr = r_copy(rule.replacement);
	foreach(tok_Node, sub_expr, new_expr) {
		if (1) {
			ast_sub(sub_expr, OLDGET(bindings, sub_expr->value.var));
			tok_destroy(sub_expr);
		}
	}


	
}
