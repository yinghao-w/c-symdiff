#include "lexer.h"
#include "../c-generics/fat_pointer.h"
#include "symbols.h"
#include <ctype.h>

int scalar_match(const char *restrict s0, const char *restrict s1) {
  /* if s is just a single non digit char, then operator or garbage */
  if (!isdigit(*s0) && s0 + 1 == s1) {
    return 0;
  }
  /* if first char is non digit and not +-, then operator or garbage */
  if (!isdigit(*s0) && *s0 != '+' && *s0 != '-') {
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

int var_match(const char *restrict s0, const char *restrict s1) {
  for (; s0 != s1; s0++) {
    if (!isalpha(*s0)) {
      return 0;
    }
  }
  return 1;
}

int opr_match(const char *restrict s0, const char *restrict s1) {
  return !!opr_get(*s0);
}

Token match(char **remainder) {
  Token token;
  char *start = *remainder;
  char *end = start;
  do {
    end++;
    if (scalar_match(start, end)) {
      char temp = *end;
      *end = '\0';
      token.token_type = SCALAR;
      token.scalar = atof(start);
      *end = temp;
      break;
    } else if (var_match(start, end)) {
      token.token_type = VAR;
      token.var = *start;
      break;
    } else if (opr_match(start, end)) {
      token.token_type = OPR;
      token.opr = opr_get(*start);
      break;
    }
  } while (!*end);
  *remainder = end;
  return token;
}

Token *lexer(char s[]) {
  Token *tokens = NULL;
  char **remainder = &s;
  while (*remainder) {
    Token token = match(remainder);
    fp_push(token, tokens);
  }
  return tokens;
}
