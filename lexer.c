#include "lexer.h"
#include "string.h"
#include "../c-generics/fat_pointer.h"
#include "symbols.h"
#include <ctype.h>
#include <stdio.h>

/* Functions which attempt to match the string from s0 inclusive to s1
 * exclusive to the corresponding type. */

static int scalar_match(const char *restrict s0, const char *restrict s1) {
  /* if s is just a single non digit char, then operator or garbage */
  if (!isdigit(*s0) && s0 + 1 == s1) {
    return 0;
  }
  /* if first char is non digit and not -, then operator or garbage */
  if (!isdigit(*s0) && *s0 != '-') {
    return 0;
  }
  /* if latter chars contain non-digits, garbage */
  for (s0++; s0 != s1; s0++) {
    if (!isdigit(*s0) && *s0 != '.') {
      return 0;
    }
  }
  return 1;
}

static int var_match(const char *restrict s0, const char *restrict s1) {
  for (; s0 != s1; s0++) {
    if (!isalpha(*s0)) {
      return 0;
    }
  }
  return 1;
}

static int opr_match(const char *restrict s0, const char *restrict s1) {
  if (s1 - s0 > 1) {
    return 0;
  } else {
    return !!opr_get(*s0);
  }
}

static void l_strip(char *remainder[]) {
  while (isspace(**remainder)) {
    (*remainder)++;
  }
}


/* Returns the atof of the string section between s0 and s1 only. In case
 * *remainder is a rvalue, copies the matched section into a temporary buffer,
 * and null terminates that section so atof scans that section only. */
static float sec_atof(const char *restrict s0, const char *restrict s1) {
	char temp[s1 - s0 + 1];
	strncpy(temp, s0, s1 - s0);
	temp[s1 - s0] = '\0';
	return atof(temp);
}

typedef enum {
  MATCH_SUCCESS,
  MATCH_ERROR,
} MATCH_CODE;

/* Attempts to match the largest string possible from *remainder onwards to a
 * token type. If successful, outputs with the token parameter and returns 0. */
static MATCH_CODE match(char *remainder[], Token *token) {
  char *start = *remainder;
  char *best = start;
  char *end = start;
  TOKEN_TYPE best_type;
  do {
    end++;
    if (opr_match(start, end)) {
      best = end;
      best_type = OPR;
    } else if (var_match(start, end)) {
      best = end;
      best_type = VAR;
    } else if (scalar_match(start, end)) {
      best = end;
      best_type = SCALAR;
    } else if (best != start) {
      break;
    }
  } while (*end);

  if (best == start) {
    return MATCH_ERROR;
  } else {

    token->token_type = best_type;

    switch (best_type) {
    case SCALAR:
	  token->scalar = sec_atof(start, best);
      break;
    case VAR:
      token->var = *start;
      break;
    case OPR:
      token->opr = opr_get(*start);
      break;
    }
    *remainder = best;
    return MATCH_SUCCESS;
  }
}

/* Add implied multiplication into a processed array of tokens, i.e.
 * 2 x -> 2 * x */
Token *mul_insert(Token tokens[]) {
  for (size_t i = 1; i < fp_length(tokens); i++) {
    if (tokens[i - 1].token_type != OPR && tokens[i].token_type != OPR) {
      Token mul = {.token_type = OPR};
      mul.opr = opr_get('*');
      fp_insert(mul, i, tokens);
    }
  }
  return tokens;
}

/* Returns a array of tokens processed from the string s. */
Token *lexer(char input[]) {
  Token *tokens = NULL;
  char **remainder = &input;
  l_strip(remainder);

  while (**remainder) {
    Token token;
    int matched = match(remainder, &token);
    if (matched == MATCH_ERROR) {
      fprintf(stderr, "Error: Could not analyse all characters.\n");
      break;
    } else {
      fp_push(token, tokens);
      l_strip(remainder);
    }
  }
  /* Reassign array location back to tokens in case it was relocated. */
  tokens = mul_insert(tokens);

  return tokens;
}
