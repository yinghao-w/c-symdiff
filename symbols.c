#include "symbols.h"
#include <string.h>

#define DPX_KT char 
#define DPX_VT Opr
#include "../trees/bad_aa.h"

#define OPR_SET_SIZE 8

ms_set opr_set = {
    {"+", 2, NULL}, {"-", 2, NULL},  {"*", 2, NULL}, {"/", 2, NULL},
    {"^", 2, NULL}, {"sq", 1, NULL}, {"(", 0, NULL}, {")", 0, NULL},
};

DPX_map *opr_set;

DPX_map *opr_set_init() {
	DPX_map *p = B_create(1);
	DPX_add("+", {2, NULL}, p);
	DPX_add("-", {2, NULL}, p);
	DPX_add("*", {2, NULL}, p);
	DPX_add("/", {2, NULL}, p);
	opr_set = p;
	return p;
}

Opr opr_get(const char s[], DPX_map *set) {
	return DPX_get (s, set)
}



Opr *get_opr(const char s[]) {
  for (int i = 0; i < OPR_SET_SIZE; i++) {
    if (strcmp(s, opr_set[i].repr) == 0) {
      return &opr_set[i];
    }
  }
  return NULL;
}

int oprcmp(const Opr *opr1, const Opr *opr2) {
  if (strcmp(opr1->repr, opr2->repr) == 0) {
    return 0;
  }

  for (int i = 0; i < OPR_SET_SIZE; i++) {
    if (strcmp(opr1->repr, opr_set[i].repr) == 0) {
      return -1;
    } else if (strcmp(opr2->repr, opr_set[i].repr) == 0) {
      return 1;
    }
  }
  return 0;
}
