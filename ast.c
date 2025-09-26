#include <ctype.h>
#include <assert.h>

#include "symbols.h"
#include "ast.h"
#include "../c-generics/fat_pointer.h"

#define fp_peek(darray) darray[fp_length(darray) - 1]

static int is_num (const char *s) {
	/* if s is just a single non digit char, then operator or garbage */
	if (!isdigit(*s) && *(s+1) == '\0') {
		return 0;
	}
	/* if first char is non digit and not +-, then operator or garbage */
	if (!isdigit(*s) && *s != '+' && *s != '-') {
		return 0;
	}
	/* if latter chars contain non-digits, garbage */
	while (*++s != '\0') {
		if (!isdigit(*s) && *s != '.') {
			return 0;
		}
	}
	return 1;
}

Token tokenify(const char s[]) {
	Token token;
	if (get_opr(s) != NULL) {
		token.token_type = OPR;
		token.opr = get_opr(s);
	} else if (is_num(s)) {
		token.token_type = SCALAR;
		token.scalar = atof(s);
	} else {
		token.token_type = VAR;
		token.var = *s;
	}
	return token;
}

/* Takes the top operator of the oprs stack, builds a tree node with value
 * operator, and arity number of children from the top of the out stack. Pushes
 * the new node onto the out stack. */
static void build(tok_Node **out, Token *oprs) {
	Token opr = fp_pop(oprs);
	tok_Node *node;
	if (opr.opr->arity == 1) {
		node = tok_join(opr, fp_pop(out), NULL);
	} else if (opr.opr->arity == 2) {
		tok_Node *rchild = fp_pop(out);
		tok_Node *lchild = fp_pop(out);
		node = tok_join(opr, lchild, rchild);
	}
	fp_push(node, out);
}

/* Shunting yard algorithm */
tok_Node *shunting_yard(char expr[]) {
	/* Initialises operator stack and output stack. Output stack consists of nodes
	 * and should be at most 2 elements always? */
	Token *oprs = NULL;
	tok_Node **out = NULL;

	/* TODO: Use safer functions */
	int len = strlen(expr);
	char *s = malloc(len + 1);
	strcpy(s, expr);
	char *str_token = strtok(s, " ");
	for (int i = 0; str_token != NULL; str_token = strtok(NULL, " ")) {
		Token token = tokenify(str_token);

		if (token.token_type != OPR) {
			tok_Node *node = tok_leaf(token);
			fp_push(node, out);

		} else if (token.token_type == OPR) {
			if (token.opr->arity == 1) {
				fp_push(token, oprs);

			} else if (token.opr->arity == 2) {
				while ((fp_length(oprs) > 0) && !(fp_peek(oprs).opr->repr[0] == '(') && (oprcmp(fp_peek(oprs).opr, token.opr) >= 0)) {
					build(out, oprs);

				}
				fp_push(token, oprs);

			} else if (token.opr->repr[0] == '(') {
				fp_push(token, oprs);

			} else if (token.opr->repr[0] == ')') {
				assert (fp_length(oprs) > 0);
				while (fp_peek(oprs).opr->repr[0] != '(') {
					build(out, oprs);

				}
				assert (fp_peek(oprs).opr->repr[0] == '(');
				fp_pop(oprs);

				if ((fp_length(oprs) > 0) && (fp_peek(oprs).opr->arity == 1)) {
					build(out, oprs);

				}
			}
		} 
	}
	while (fp_length(oprs) > 0) {
		assert (fp_peek(oprs).opr->repr[0] != '(');
		build(out, oprs);
	}
	free(s);
	fp_destroy(oprs);
	tok_Node *root = fp_pop(out);
	fp_destroy(out);
	return root;
}

void ast_sub(tok_Node *old_node, tok_Node *new_node) {
	if (old_node->parent == NULL) {
		return;
	}
	if (old_node == old_node->parent->lchild) {
		old_node->parent->lchild = new_node;
	} else if (old_node == old_node->parent->rchild) {
		old_node->parent->rchild = new_node;
	}
	new_node->parent = old_node->parent;
	old_node->parent = NULL;
}
